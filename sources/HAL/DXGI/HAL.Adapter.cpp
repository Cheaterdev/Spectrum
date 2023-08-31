 module HAL:Adapter;
 import <HAL.h>;
 import <d3d12/d3d12_includes.h>;
 import windows;
 import streamline;
 namespace HAL
 {

		Adapters::Adapters()
		{
			sl::Preferences pref{};

			pref.showConsole = true;                        // for debugging, set to false in production
			//pref.logLevel = sl::eLogLevelDefault;
			pref.applicationId = 1; // Provided by NVDA, required if using NGX components (DLSS 2/3)
//pref.engineType = sl::EngineType::eCustom; // If using UE or Unity
pref.engineVersion = "ver 0.1"; // Optional version
pref.projectId = "Spectrum"; // Optional project id

			 wchar_t path[FILENAME_MAX] = { 0 };
        GetModuleFileNameW(nullptr, path, FILENAME_MAX);

		std::filesystem::path fp = path;

		
        std::wstring s =fp.parent_path().wstring();

		s+= L"\\Streamline\\";



			const wchar_t* streamlinePath =s.c_str();
			pref.pathsToPlugins = {&streamlinePath}; // change this if Streamline plugins are not located next to the executable
			pref.numPathsToPlugins = 1; // change this if Streamline plugins are not located next to the executable


//
			sl::Feature myFeatures[] = { sl::kFeatureNRD, sl::kFeatureDLSS };
pref.featuresToLoad = myFeatures;
pref.numFeaturesToLoad = _countof(myFeatures);

			// Inform SL that it is OK to use newer version of SL or NGX (if available)
			//pref.flags |= sl::Preferences::PreferenceFlag::eAllowOTA;
			// Set other preferences, request features etc.
			auto res = slInit(pref);


		//	 auto mod = LoadLibrary("sl.interposer.dll");

  //  // These are the exports from SL library
  //  typedef HRESULT(WINAPI* PFunCreateDXGIFactory)(REFIID, void**);
  //  typedef HRESULT(WINAPI* PFunCreateDXGIFactory1)(REFIID, void**);
  //  typedef HRESULT(WINAPI* PFunCreateDXGIFactory2)(UINT, REFIID, void**);
  //  typedef HRESULT(WINAPI* PFunDXGIGetDebugInterface1)(UINT, REFIID, void**);
  //  typedef HRESULT(WINAPI* PFunD3D12CreateDevice)(IUnknown* , D3D_FEATURE_LEVEL, REFIID , void**);

  //  // Map functions from SL and use them instead of standard DXGI/D3D12 API
  //  auto slCreateDXGIFactory = reinterpret_cast<PFunCreateDXGIFactory>(GetProcAddress(mod, "CreateDXGIFactory"));
  //  auto slCreateDXGIFactory1 = reinterpret_cast<PFunCreateDXGIFactory1>(GetProcAddress(mod, "CreateDXGIFactory1"));
  //  auto slCreateDXGIFactory2 = reinterpret_cast<PFunCreateDXGIFactory2>(GetProcAddress(mod, "CreateDXGIFactory2"));
  //  auto slDXGIGetDebugInterface1 = reinterpret_cast<PFunDXGIGetDebugInterface1>(GetProcAddress(mod, "DXGIGetDebugInterface1"));
  //  auto slD3D12CreateDevice = reinterpret_cast<PFunD3D12CreateDevice>(GetProcAddress(mod, "D3D12CreateDevice"));



		//slCreateDXGIFactory2(0, IID_PPV_ARGS(&native_factory));
			auto hr=    	CreateDXGIFactory2(0, IID_PPV_ARGS(&native_factory));
		}
	 }