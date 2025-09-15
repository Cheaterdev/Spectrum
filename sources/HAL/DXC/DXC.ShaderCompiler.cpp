
module HAL:ShaderCompiler;
import wrl;
import Core;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;

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

		compilationArguments.emplace_back(L"-no-warnings");
		compilationArguments.emplace_back(L"-O3");

		if (check(options & ShaderOptions::FP16))
		{
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


		if (entry_point.size())
		{

			blob_str.functions.emplace_back();
			auto& f = blob_str.functions.back();

			ComPtr<ID3D12ShaderReflection> shaderReflection{};
			library->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
			D3D12_SHADER_DESC shaderDesc{};
			shaderReflection->GetDesc(&shaderDesc);
			f.name = entry_point;
								  f.wname = convert(f.name);
			for (int i = 0; i < shaderDesc.ConstantBuffers; i++)
			{
				ID3D12ShaderReflectionConstantBuffer* cb = shaderReflection->GetConstantBufferByIndex(i);
				D3D12_SHADER_BUFFER_DESC shaderBufferDesc{};
				cb->GetDesc(&shaderBufferDesc);

				std::string cb_name = shaderBufferDesc.Name;

				if (cb_name.starts_with("pass_"))
				{
					cb_name = cb_name.substr(5);
					auto slot_id = get_slot(cb_name);

					if(slot_id)
					f.slots.merge(slot_id.value());
				}

				//Log::get() << shaderBufferDesc.Name << Log::endl;

			}

		} else
		{
		   	ComPtr<ID3D12LibraryReflection> libraryReflection{};
			auto hr3 = library->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&libraryReflection));


			D3D12_LIBRARY_DESC shaderDesc{};
			libraryReflection->GetDesc(&shaderDesc);
				

				for (int i = 0; i < shaderDesc.FunctionCount; i++)
			{
				ID3D12FunctionReflection* f = libraryReflection->GetFunctionByIndex(i);
				D3D12_FUNCTION_DESC functionDesc{};
				f->GetDesc(&functionDesc);
		//		Log::get() << "FUNCTION "<< functionDesc.Name << Log::endl;

				blob_str.functions.emplace_back();
					auto& rf = blob_str.functions.back();
						 	  rf.name = functionDesc.Name ;
							  rf.wname = convert(rf.name);


											 	for (int i = 0; i < functionDesc.ConstantBuffers; i++)
			{
				ID3D12ShaderReflectionConstantBuffer* cb = f->GetConstantBufferByIndex(i);
				D3D12_SHADER_BUFFER_DESC shaderBufferDesc{};
				cb->GetDesc(&shaderBufferDesc);

						   	std::string cb_name = shaderBufferDesc.Name;

			
													if (cb_name.starts_with("pass_"))
				{
					cb_name = cb_name.substr(5);

					auto slot_id = get_slot(cb_name);

					rf.slots.merge(slot_id.value());
				}
			//	Log::get() << shaderBufferDesc.Name << Log::endl;

			}
			}


		}



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
