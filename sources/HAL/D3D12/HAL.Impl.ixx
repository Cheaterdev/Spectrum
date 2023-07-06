export module HAL:Impl;
import <d3d12/d3d12_includes.h>;
import :Debug;
import wrl;

export namespace HAL
{

	void EnableGPUDebug()
	{

		ComPtr<ID3D12DeviceRemovedExtendedDataSettings> pDredSettings;
		(D3D12GetDebugInterface(IID_PPV_ARGS(&pDredSettings)));

		if (pDredSettings)
		{
			pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);

		}

		ComPtr<ID3D12Debug> debugController;
		ComPtr<ID3D12Debug1> spDebugController1;

#ifdef DEV
		if constexpr (Debug::CheckErrors)
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->QueryInterface(IID_PPV_ARGS(&spDebugController1));


				debugController->EnableDebugLayer();
				//	spDebugController1->SetEnableGPUBasedValidation(true);
			}
#endif

	}

	void EnableShaderModel()
	{
		const UUID _D3D12ExperimentalShaderModels = { /* 76f5573e-f13a-40f5-b297-81ce9e18933f */
0x76f5573e,
0xf13a,
0x40f5,
{ 0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f }
		};

		D3D12EnableExperimentalFeatures(1, &_D3D12ExperimentalShaderModels, nullptr, nullptr);

	}


	void init()
	{
		EnableGPUDebug();
		EnableShaderModel();

	}
}


