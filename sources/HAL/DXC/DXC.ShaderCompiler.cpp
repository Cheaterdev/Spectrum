
module HAL:ShaderCompiler;
import wrl;
import Core;

import windows;       // COM base types (IUnknown, INoMarshal), MessageBoxA, LPCWSTR
import DXCompiler;
import crc32c;

// Shader reflection is the only D3D12-coupled part of compilation (it uses the
// ID3D12ShaderReflection / ID3D12LibraryReflection interfaces).  It is split out
// behind this seam so the common compile path stays backend-neutral:
//   * D3D12  build: D3D12/HAL.D3D12.ShaderReflection.cpp  (real reflection)
//   * Vulkan build: Vulkan/HAL.Vulkan.ShaderReflection.cpp (stub for now)
// Both are implementation units of `module HAL:ShaderCompiler`, so this
// declaration has module linkage and is visible to whichever one is compiled.
namespace HAL
{
    void reflect_shader(IDxcUtils* library, const DxcBuffer& reflectionBuffer,
                        const std::string& entry_point, CompiledShader& out);

    // Backend-specific extra DXC compilation flags.
    // D3D12 backend: returns {} (no-op).
    // Vulkan backend: returns { L"-spirv", L"-fvk-use-dx-layout", ... }.
    std::vector<std::wstring> get_extra_compile_args(const std::string& target);
}

#define DXC_MICROCOM_REF_FIELD(m_dwRef)                                        \
  volatile std::atomic_int m_dwRef = {0};
#define DXC_MICROCOM_ADDREF_IMPL(m_dwRef)                                      \
  ULONG STDMETHODCALLTYPE AddRef() override {                                  \
    return (ULONG)++m_dwRef;                                                   \
  }
#define DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)                              \
  DXC_MICROCOM_ADDREF_IMPL(m_dwRef)                                            \
  ULONG STDMETHODCALLTYPE Release() override {                                 \
    ULONG result = (ULONG)--m_dwRef;                                           \
    if (result == 0)                                                           \
      delete this;                                                             \
    return result;                                                             \
  }



	template<typename TObject>
	HRESULT DoBasicQueryInterface_recurse(TObject* self, REFIID iid, void** ppvObject) {
		return E_NOINTERFACE;
	}
	template<typename TObject, typename TInterface, typename... Ts>
	HRESULT DoBasicQueryInterface_recurse(TObject* self, REFIID iid, void** ppvObject) {
		if (ppvObject == nullptr) return E_POINTER;
		if (IsEqualIID(iid, __uuidof(TInterface))) {
			*(TInterface**)ppvObject = self;
			self->AddRef();
			return S_OK;
		}
		return DoBasicQueryInterface_recurse<TObject, Ts...>(self, iid, ppvObject);
	}
	template<typename... Ts, typename TObject>
	HRESULT DoBasicQueryInterface(TObject* self, REFIID iid, void** ppvObject) {
		if (ppvObject == nullptr) return E_POINTER;

		// Support INoMarshal to void GIT shenanigans.
		if (IsEqualIID(iid, __uuidof(IUnknown)) ||
			IsEqualIID(iid, __uuidof(INoMarshal))) {
			*ppvObject = reinterpret_cast<IUnknown*>(self);
			reinterpret_cast<IUnknown*>(self)->AddRef();
			return S_OK;
		}

		return DoBasicQueryInterface_recurse<TObject, Ts...>(self, iid, ppvObject);
	}



namespace HAL
{

	namespace API
	{


		class shader_include_dxil : public IDxcIncludeHandler
		{
			HAL::shader_include* includer;
			std::set<std::unique_ptr<std::string>> includes;
		private:
			DXC_MICROCOM_REF_FIELD(m_dwRef)
		public:
			shader_include_dxil(HAL::shader_include* includer) : includer(includer)
			{

			}

			DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
				virtual ~shader_include_dxil() {}
			HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) {
				return DoBasicQueryInterface<::IDxcIncludeHandler>(this, riid, ppvObject);
			}


			virtual HRESULT STDMETHODCALLTYPE LoadSource(
				_In_ LPCWSTR pFilename,                                   // Candidate filename.
				_COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource  // Resultant source object for included file, nullptr if not found.
			) override
			{
				if (!includer)
				{
					return E_FAIL;
				}

				auto data = includer->load_file(convert(pFilename));

				if (!data)
				{
					return E_FAIL;
				}

				auto ppData = reinterpret_cast<const void*>(data->data());
				auto pBytes = static_cast<unsigned int>(data->size());
				includes.insert(std::move(data));
				ComPtr<IDxcBlobEncoding> pEncodingIncludeSource;
				HAL::ShaderCompiler::get().library->CreateBlobFromPinned((LPBYTE)ppData, pBytes, CP_ACP, &pEncodingIncludeSource);
				*ppIncludeSource = pEncodingIncludeSource.Detach();
				return S_OK;
			}

		};
	}


	// Rewrites every Log("format string", ...) call in already-preprocessed
	// HLSL text (macros expanded, includes flattened -- see the preprocess
	// pass in Compile_Shader) into Log(<id>u, ...), where id is a hash of
	// the format string, and registers {id, format string} with
	// DebugLogStrings for CommandList::print_debug() to format on readback.
	//
	// This can only run on *preprocessed* text. A raw string literal isn't a
	// valid argument to any Log() overload, so a Log("...") call reached
	// through a macro would never resolve if we scanned each #include'd
	// file's raw text separately (macros aren't expanded per-file, only
	// once DXC assembles and preprocesses the whole translation unit) --
	// and running a real (type-checking) compile on the *original* source to
	// find these calls isn't an option either, since it would just fail on
	// the string literal before we get the chance to rewrite it.
	//
	// found_formats collects this call's own discoveries (as well as
	// registering each one into the live DebugLogStrings singleton
	// immediately) so the caller can attach them to the CompiledShader it
	// returns -- the shader cache (Shader<T>::SERIALIZE, HAL.Shader.ixx) can
	// load a cached blob straight from disk without ever calling
	// Compile_Shader again, and DebugLogStrings is in-memory only, so
	// without persisting them here too a cache hit would leave every Log()
	// readback showing "unregistered log id" instead of the real message.
	static std::string rewrite_debug_log_strings(std::string text, std::unordered_map<uint32_t, std::string>& found_formats)
	{
		const std::string needle = "Log(";
		if (!text.contains(needle))
			return text;

		std::string result;
		result.reserve(text.size());

		size_t pos = 0;
		while (true)
		{
			size_t call = text.find(needle, pos);
			if (call == std::string::npos)
			{
				result.append(text, pos, text.size() - pos);
				break;
			}

			size_t quote = call + needle.size();
			while (quote < text.size() && (text[quote] == ' ' || text[quote] == '\t' || text[quote] == '\n' || text[quote] == '\r'))
				++quote;

			if (quote >= text.size() || text[quote] != '"')
			{
				// Not a Log("...") call -- a differently-typed first arg, or
				// an unrelated function that happens to also be named Log.
				// Leave it untouched and keep scanning past "Log(".
				result.append(text, pos, quote - pos);
				pos = quote;
				continue;
			}

			size_t str_end = quote + 1;
			while (str_end < text.size() && text[str_end] != '"')
				str_end += (text[str_end] == '\\' && str_end + 1 < text.size()) ? 2 : 1;

			if (str_end >= text.size())
			{
				// Unterminated string literal -- leave as-is; the real
				// compile pass will report the actual syntax error.
				result.append(text, pos, text.size() - pos);
				break;
			}

			std::string fmt = text.substr(quote + 1, str_end - quote - 1);
			uint32_t id = crc32c::Crc32c(fmt);
			DebugLogStrings::get().register_format(id, fmt);
			found_formats.try_emplace(id, std::move(fmt));

			result.append(text, pos, call - pos);
			result += "Log(";
			result += std::to_string(id);
			result += "u";

			pos = str_end + 1; // resume right after the closing quote
		}

		return result;
	}

	// True once `text` contains DebugInfo's own accessor -- pulled in
	// either directly (a shader that calls GetDebugInfo() itself, old
	// style) or transitively through something else the shader includes
	// (e.g. any table bound under the Frame::FrameLayout tree the DebugInfo slot
	// itself lives in). Its own definition, wherever it came from, is
	// necessarily the first occurrence of the text "GetDebugInfo()" --
	// anything else referencing it, this shim included, has to already be
	// textually below it (DXC resolves identifiers top-to-bottom like C).
	static bool has_debug_info(const std::string& text)
	{
		return text.contains("GetDebugInfo()");
	}

	// Gives shader authors a bare Log("fmt", args...) call -- no
	// GetDebugInfo() prefix, no manual #include, no manual asuint() -- by
	// forwarding to DebugInfo's own member Log() overloads
	// (sources/Prism/defs/defaultlayout.prism). Called on text that
	// Compile_Shader has already made sure contains "GetDebugInfo()" (see
	// has_debug_info and the retry-with-#include-then-reflatten path
	// around the preprocess pass below) -- inserting the wrapper is the
	// easy, purely-textual part; getting DebugInfo.h into the flattened
	// text safely in the first place is the part that needs a real
	// preprocess pass, not string surgery -- see the comment there.
	static std::string insert_debug_log_wrapper(std::string text)
	{
		// LogArg + templated Log(id, T0, T1, ...) (HLSL 2021 -- see -HV 2021
		// in compilationArguments below): lets a call site pass a typed
		// value directly, Log("dist=%f", dist), instead of requiring
		// asuint(dist) at every call. LogArg is what actually picks the
		// bit-reinterpretation per argument type; the %f/%u/%d/%x in the
		// format string still separately drives how the CPU side re-reads
		// those bits at print time -- LogArg only has to get the bits
		// *in*, not know what they mean.
		//
		// LogWrite4 exists because ConstantBuffer<T>'s transparent member
		// forwarding (GetDebugInfo().Log(...) working as if GetDebugInfo()
		// returned a DebugInfo directly, not a ConstantBuffer<DebugInfo>)
		// does not participate correctly in name lookup from inside a
		// template body -- DXC rejects it there ("no known conversion from
		// 'ConstantBuffer<DebugInfo>' to 'DebugInfo'") even though the
		// identical call compiles fine in ordinary, non-generic code. The
		// templates below never touch GetDebugInfo() directly; they only
		// ever call this one plain (non-template) function that does.
		static const std::string wrapper =
			"uint LogArg(uint v) { return v; }\n"
			"uint LogArg(int v) { return uint(v); }\n"
			"uint LogArg(float v) { return asuint(v); }\n"
			"void LogWrite4(uint id, uint4 args) { GetDebugInfo().Log(id, args.x, args.y, args.z, args.w); }\n"
			"void Log(uint id) { LogWrite4(id, uint4(0, 0, 0, 0)); }\n"
			"template<typename T0>\n"
			"void Log(uint id, T0 a0) { LogWrite4(id, uint4(LogArg(a0), 0, 0, 0)); }\n"
			"template<typename T0, typename T1>\n"
			"void Log(uint id, T0 a0, T1 a1) { LogWrite4(id, uint4(LogArg(a0), LogArg(a1), 0, 0)); }\n"
			"template<typename T0, typename T1, typename T2>\n"
			"void Log(uint id, T0 a0, T1 a1, T2 a2) { LogWrite4(id, uint4(LogArg(a0), LogArg(a1), LogArg(a2), 0)); }\n"
			"template<typename T0, typename T1, typename T2, typename T3>\n"
			"void Log(uint id, T0 a0, T1 a1, T2 a2, T3 a3) { LogWrite4(id, uint4(LogArg(a0), LogArg(a1), LogArg(a2), LogArg(a3))); }\n";

		size_t def = text.find("GetDebugInfo()");
		ASSERT(def != std::string::npos && "insert_debug_log_wrapper called before DebugInfo.h was confirmed present");
		if (def == std::string::npos)
			return text;

		size_t line_end = text.find('\n', def);
		size_t insert_pos = (line_end == std::string::npos) ? text.size() : line_end + 1;
		text.insert(insert_pos, wrapper);
		return text;
	}

	// Preprocess-only pass (-P): DXC expands every macro and flattens every
	// #include into one string, with NO semantic/type checking -- so a
	// Log("format string", ...) call reached through any number of macro
	// layers or nested includes shows up here as plain literal text exactly
	// once, and a bare string-literal argument (which the real compile
	// would reject -- no Log() overload takes a string) causes no error at
	// this stage. See DXC_OUT_HLSL in dxcapi.h ("Compile() with -P").
	// Returns nullopt (having already logged/shown the error, matching
	// Compile_Shader's existing error path) on failure.
	static std::optional<std::string> run_preprocess_pass(IDxcCompiler3* compiler, const std::string& text,
		const std::vector<LPCWSTR>& base_args, IDxcIncludeHandler* include_handler, const std::string& file_name)
	{
		DxcBuffer buffer{ .Ptr = text.data(), .Size = text.size(), .Encoding = CP_UTF8 };

		std::vector<LPCWSTR> args = base_args;
		args.push_back(L"-P");

		Microsoft::WRL::ComPtr<IDxcResult> result{};
		HRESULT hr = compiler->Compile(&buffer, args.data(), static_cast<uint32_t>(args.size()), include_handler, IID_PPV_ARGS(&result));

		result->GetStatus(&hr);
		if (FAILED(hr))
		{
			IDxcBlobEncoding* error;
			result->GetErrorBuffer(&error);

			std::string infoLog;
			infoLog.assign(static_cast<const char*>(error->GetBufferPointer()), static_cast<const char*>(error->GetBufferPointer()) + error->GetBufferSize());

			std::string errorMsg = "Shader Preprocess Error:\n";
			errorMsg += file_name + "\n";
			errorMsg.append(infoLog);
			Log::get() << Log::LEVEL_ERROR << errorMsg << Log::endl;

			MessageBoxA(nullptr, errorMsg.c_str(), "Error!", MB_OK);
			return std::nullopt;
		}

		ComPtr<IDxcBlob> blob;
		result->GetOutput(DXC_OUT_HLSL, IID_PPV_ARGS(&blob), nullptr);
		return std::string(static_cast<const char*>(blob->GetBufferPointer()), blob->GetBufferSize());
	}

	std::optional<CompiledShader>  ShaderCompiler::Compile_Shader_File(std::string filename, std::vector < HAL::shader_macro> macros, std::string target, std::string entry_point, ShaderOptions options, HAL::shader_include* includer)
	{
		auto data = includer->load_file(filename);
		return Compile_Shader(*data, macros, target, entry_point, options, includer, filename);
	}
	//

	std::optional<CompiledShader>  ShaderCompiler::Compile_Shader(std::string shaderText, std::vector < HAL::shader_macro> macros, std::string target, std::string entry_point, ShaderOptions options, HAL::shader_include* includer, std::string file_name)
	{

		if (file_name.empty())
			file_name = "shaders/unknown";

		// Kept around for the debug-log #include retry below: needs to
		// re-preprocess the *original* source with an #include added, not
		// the already-flattened text shaderText gets reassigned to further
		// down (see the comment there for why).
		const std::string original_shader_text = shaderText;

		resource_file_depender dep;
		HAL::shader_include in(file_name, dep);

		if (!includer)includer = &in;


		static std::mutex m;
		std::lock_guard<std::mutex> g(m);
		std::vector<std::wstring> vdefines;
		std::vector<LPCWSTR> vargs;

		std::vector<DxcDefine> defines;

		for (auto& m : macros)
		{
			vdefines.push_back(convert(m.name));
			vdefines.push_back(convert(m.value));

		}

		int i = 0;
		for (auto& m : macros)	defines.push_back(DxcDefine{ vdefines[i++].c_str(), vdefines[i++].c_str() });

		std::wstring build_macro = L"BUILD_FUNC_" + convert(entry_point);
		std::wstring build_def = L"1";

		defines.push_back(DxcDefine{ build_macro.c_str(), build_def.c_str() });

		std::wstring no_macro = L"NO_GLOBAL";
		std::wstring no_def = L"1";

		if (target.find("lib") == 0)
		{
			defines.push_back(DxcDefine{ no_macro.c_str(), no_def.c_str() });
		}


		DxcBuffer sourceBuffer
		{
			.Ptr = shaderText.data(),
			.Size = shaderText.size(),
			.Encoding = CP_UTF8,
		};
		std::vector<std::wstring> compilationArguments;



		if (entry_point.size())
		{
			compilationArguments.emplace_back(L"-E");
			compilationArguments.emplace_back(convert(entry_point));

		}
		compilationArguments.emplace_back(L"-T");
		compilationArguments.emplace_back(convert(target));

		compilationArguments.emplace_back(L"-HV");
		compilationArguments.emplace_back(L"2021");

		compilationArguments.emplace_back(L"-no-warnings");
		compilationArguments.emplace_back(L"-O3");

		// Backend-specific extra flags (e.g. "-spirv" for Vulkan).
		// Defined in D3D12/HAL.D3D12.ShaderReflection.cpp (returns {})
		// and Vulkan/HAL.Vulkan.ShaderReflection.cpp (returns SPIR-V flags).
		for (auto& extra : get_extra_compile_args(target))
			compilationArguments.push_back(extra);

		{
			// -enable-16bit-types makes `half` a native 16-bit type in DXIL instead of a
			// min-precision alias.  Without it, the DXIL validator rejects bitcast operations
			// on `half` (e.g. in FFX denoiser shaders that use asuint(half)).
			// For SPIRV targets we must NOT set this flag: DXC would emit native
			// OpTypeImage %half which violates VUID-StandaloneSpirv-OpTypeImage-04656
			// (sampled-image component type must be 32-bit).  The Vulkan shaders instead
			// handle float16 via explicit #ifdef __spirv__ + float16_t aliases.
			bool is_spirv = std::any_of(compilationArguments.begin(), compilationArguments.end(),
				[](const std::wstring& a) { return a == L"-spirv"; });
			if (!is_spirv)
				compilationArguments.push_back(L"-enable-16bit-types");
		}

		for (auto& m : defines)
		{
			compilationArguments.push_back(std::wstring(L"-D") + std::wstring(m.Name) + L"=" + m.Value);
		}
		std::vector<LPCWSTR> nativeCompilationArguments;


		for (auto& e : compilationArguments)
		{
			nativeCompilationArguments.emplace_back(e.c_str());
		}


		API::shader_include_dxil dxil_include(includer);
		dxil_include.AddRef();

		std::unordered_map<uint32_t, std::string> debug_log_formats;

		// rewrite_debug_log_strings needs already-preprocessed text (see its
		// own comment for why), so run the preprocess pass, rewrite any
		// Log("...") calls it finds into Log(<id>u, ...), and -- if any
		// were found and DebugInfo.h isn't already part of the flattened
		// text -- get it in there and insert the small forwarding shim
		// (LogArg/LogWrite4/templated Log(), see insert_debug_log_wrapper)
		// before the real compile below ever sees the result.
		{
			auto preprocessed = run_preprocess_pass(compiler, shaderText, nativeCompilationArguments, &dxil_include, file_name);
			if (!preprocessed)
				return {};

			shaderText = rewrite_debug_log_strings(std::move(*preprocessed), debug_log_formats);

			if (!debug_log_formats.empty() && !has_debug_info(shaderText))
			{
				// Don't splice #include "autogen/DebugInfo.h" straight into
				// this already-flattened text and call it done: that starts
				// a second, independent preprocessing context in the real
				// compile pass below, which has no memory of what the first
				// -P pass already resolved -- so anything DebugInfo.h pulls
				// in (its own layout, e.g.) that the shader *also* already
				// depends on some other way ends up defined twice, same
				// class of bug as calling GetDebugInfo() before it exists
				// (see insert_debug_log_wrapper), just one level removed and
				// on a dependency instead of DebugInfo.h itself. Preprocess
				// the *original* source again with the #include added, so
				// the one unified -P pass naturally dedupes everything the
				// normal way (the same way a shader manually writing that
				// #include itself already works).
				std::string augmented = "#include \"autogen/DebugInfo.h\"\n" + original_shader_text;
				auto reflattened = run_preprocess_pass(compiler, augmented, nativeCompilationArguments, &dxil_include, file_name);
				if (!reflattened)
					return {};

				shaderText = rewrite_debug_log_strings(std::move(*reflattened), debug_log_formats);
			}

			if (!debug_log_formats.empty())
				shaderText = insert_debug_log_wrapper(std::move(shaderText));

			sourceBuffer.Ptr = shaderText.data();
			sourceBuffer.Size = shaderText.size();
		}

		// Compile the shader.
		Microsoft::WRL::ComPtr<IDxcResult> compiledShaderBuffer{};
		HRESULT hr = compiler->Compile(&sourceBuffer,
			nativeCompilationArguments.data(),
			static_cast<uint32_t>(nativeCompilationArguments.size()),
			&dxil_include,
			IID_PPV_ARGS(&compiledShaderBuffer));


		// Verify the result
		compiledShaderBuffer->GetStatus(&hr);
		if (FAILED(hr))
		{
			IDxcBlobEncoding* error;
			hr = compiledShaderBuffer->GetErrorBuffer(&error);

			std::string infoLog;
			infoLog.assign(static_cast<const char*>(error->GetBufferPointer()), static_cast<const char*>(error->GetBufferPointer()) + error->GetBufferSize());

			std::string errorMsg = "Shader Compiler Error:\n";
			errorMsg += file_name + "\n";
			errorMsg.append((infoLog));
			Log::get() << Log::LEVEL_ERROR << errorMsg << Log::endl;
		
			MessageBoxA(nullptr, errorMsg.c_str(), "Error!", MB_OK);
			return {};
		}
		ComPtr<IDxcBlob> resultBlob;
		compiledShaderBuffer->GetResult(&resultBlob);

		CompiledShader blob_str;
		blob_str.blob.assign(static_cast<std::byte*>(resultBlob->GetBufferPointer()), static_cast<std::byte*>(resultBlob->GetBufferPointer()) + resultBlob->GetBufferSize());


		ComPtr<IDxcBlob> reflectionBlob{};
		compiledShaderBuffer->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr);

		const DxcBuffer reflectionBuffer
		{
			.Ptr = reflectionBlob->GetBufferPointer(),
			.Size = reflectionBlob->GetBufferSize()
		};

		// Backend-specific: D3D12 uses ID3D12ShaderReflection to extract per-pass
		// constant-buffer slot usage; Vulkan stubs this for now (Phase 4 will use
		// SPIR-V reflection).  See reflect_shader() seam at top of this TU.
		reflect_shader(library, reflectionBuffer, entry_point, blob_str);
		blob_str.entry_point = entry_point; // store so pipeline creation can use it
		blob_str.debug_log_formats = std::move(debug_log_formats);
		return std::move(blob_str);

	}


	const CLSID _CLSID_DxcCompiler = {
	   0x73e22d93,
	   0xe6ce,
	   0x47f3,
	   {0xb5, 0xbf, 0xf0, 0x66, 0x4f, 0x39, 0xc1, 0xb0} };

	// {6245D6AF-66E0-48FD-80B4-4D271796748C}
	const GUID _CLSID_DxcLibrary = {
	   0x6245d6af,
	   0x66e0,
	   0x48fd,
	   {0x80, 0xb4, 0x4d, 0x27, 0x17, 0x96, 0x74, 0x8c} };

	ShaderCompiler::ShaderCompiler()
	{

		auto hr = DxcDllHelper.Initialize();
		//	Utils::Validate(hr, L"Failed to initialize DxCDllSupport!");

		//::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))
		DxcDllHelper.CreateInstance(_CLSID_DxcCompiler, &compiler);
		//	Utils::Validate(hr, L"Failed to create DxcCompiler!");

		DxcDllHelper.CreateInstance(_CLSID_DxcLibrary, &library);
		//	Utils::Validate(hr, L"Failed to create DxcLibrary!");
		\


	}
}
