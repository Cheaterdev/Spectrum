
module HAL:Shaders;

import Singleton;
import FileSystem;
import FileDepender;
import DXCompiler;
import serialization;
import Log;

namespace D3D
{

	shader_macro::shader_macro(std::string name, std::string value /*= "1"*/)
	{
		this->name = name;
		this->value = value;
	}




	shader_include::shader_include(std::string dir, resource_file_depender& _depender) : depender(_depender)
	{

		std::transform(dir.begin(), dir.end(), dir.begin(), [](char s) {
			if (s == '/') return '\\';
			return s;
			});
		this->dir = dir.substr(0, dir.find_last_of('\\')) + '\\';
	}

	std::unique_ptr<std::string> shader_include::load_file(std::string pFileName)
	{
		auto sig = pFileName.find("autogen");
		if (sig != std::string::npos)
		{
			autogen.emplace_back(pFileName);
		}

		std::string file_name = dir + pFileName;
		auto file = FileSystem::get().get_file(convert(file_name));


		if (!file) file = FileSystem::get().get_file(convert(pFileName));
		if (!file)
		{
			return nullptr;
		}

		auto data = file->load_all();
		depender.add_depend(file);

		return std::make_unique<std::string>(std::move(data));

	}

}

class shader_include_d3d : public ID3DInclude
{
	D3D::shader_include* includer;

public:


	shader_include_d3d(D3D::shader_include* includer) : includer(includer)
	{

	}


	STDMETHODIMP Close(LPCVOID pData)
	{
		return S_OK;
	}

	STDMETHODIMP Open(D3D_INCLUDE_TYPE includeType, LPCSTR pFileName, LPCVOID /*pParentData*/, LPCVOID* ppData, UINT* pBytes)
	{

		if (!includer)
		{
			return E_FAIL;
		}

		auto data = includer->load_file(pFileName);

		if (!data)
		{
			return E_FAIL;
		}

		*ppData = reinterpret_cast<const void*>(data->data());
		*pBytes = static_cast<unsigned int>(data->size());
		return S_OK;
	}


};
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


class shader_include_dxil : public IDxcIncludeHandler
{
	D3D::shader_include* includer;
	std::set<std::unique_ptr<std::string>> includes;
private:
	DXC_MICROCOM_REF_FIELD(m_dwRef)
public:
	shader_include_dxil(D3D::shader_include* includer) : includer(includer)
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
		ShaderCompiler::get().library->CreateBlobWithEncodingFromPinned((LPBYTE)ppData, pBytes, CP_ACP, &pEncodingIncludeSource);
		*ppIncludeSource = pEncodingIncludeSource.Detach();
		return S_OK;
	}

};



std::optional<binary>  ShaderCompiler::Compile_Shader_File(std::string filename, std::vector < D3D::shader_macro> macros, std::string target, std::string entry_point, D3D::shader_include* includer)
{
	auto data = includer->load_file(filename);
	return Compile_Shader(*data, macros, target, entry_point, includer, filename);
}
//

std::optional<binary>  ShaderCompiler::Compile_Shader(std::string shaderText, std::vector < D3D::shader_macro> macros, std::string target, std::string entry_point, D3D::shader_include* includer, std::string file_name)
{

	if (file_name.empty())
		file_name = "shaders/unknown";
	resource_file_depender dep;
	D3D::shader_include in(file_name, dep);

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

	HRESULT hr;
	UINT32 code(0);
	IDxcBlobEncoding* pSource(nullptr);

	library->CreateBlobWithEncodingFromPinned((LPBYTE)shaderText.data(), (UINT)shaderText.size(), CP_UTF8, &pSource);


	shader_include_dxil dxil_include(includer);
	dxil_include.AddRef();
	IDxcOperationResult* result;

	vargs.push_back(L"-no-warnings");
	vargs.push_back(L"-O3");

	hr = compiler->Compile(
		pSource,          // program text
		convert(file_name).c_str(),   // file name, mostly for error messages
		convert(entry_point).c_str(),          // entry point function
		convert(target).c_str(),        // target profile
		vargs.data(),           // compilation arguments
		(UINT)vargs.size(), // number of compilation arguments
		defines.data(), (UINT)defines.size(),    // name/value defines and their count
		&dxil_include,          // handler for #include directives
		&result);

	// Verify the result
	result->GetStatus(&hr);
	if (FAILED(hr))
	{
		IDxcBlobEncoding* error;
		hr = result->GetErrorBuffer(&error);

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
	result->GetResult(&resultBlob);

	binary blob_str;
	blob_str.assign(static_cast<std::byte*>(resultBlob->GetBufferPointer()), static_cast<std::byte*>(resultBlob->GetBufferPointer()) + resultBlob->GetBufferSize());

	return std::move(blob_str);

}

const CLSID _CLSID_DxcCompiler = {
   0x73e22d93,
   0xe6ce,
   0x47f3,
   {0xb5, 0xbf, 0xf0, 0x66, 0x4f, 0x39, 0xc1, 0xb0} };

// {EF6A8087-B0EA-4D56-9E45-D07E1A8B7806}
const GUID _CLSID_DxcLibrary = {
   0x6245d6af,
   0x66e0,
   0x48fd,
   {0x80, 0xb4, 0x4d, 0x27, 0x17, 0x96, 0x74, 0x8c} };


ShaderCompiler::ShaderCompiler()
{
	auto hr = DxcDllHelper.Initialize();
	//	Utils::Validate(hr, L"Failed to initialize DxCDllSupport!");

	DxcDllHelper.CreateInstance(_CLSID_DxcCompiler, &compiler);
	//	Utils::Validate(hr, L"Failed to create DxcCompiler!");

	DxcDllHelper.CreateInstance(_CLSID_DxcLibrary, &library);
	//	Utils::Validate(hr, L"Failed to create DxcLibrary!");
}