module HAL:Impl;

import :Debug;
import :Streamline;
import d3d12;
import wrl;

namespace HAL
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

        if constexpr (Debug::CheckErrors)
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->QueryInterface(IID_PPV_ARGS(&spDebugController1));
                debugController->EnableDebugLayer();

                // GPU-based validation. Off by default -- it costs a large
                // multiple of frame time, so it is a deliberate session, not a
                // background setting.
                //
                // Worth turning on for the class of bug the CPU debug layer
                // structurally cannot see: it validates the DESCRIPTOR a shader
                // actually dereferences against the resource's live layout, which
                // is how #1358 (UAV descriptor against a SHADER_RESOURCE layout)
                // and #939/#940 (unbound slot or member reading descriptor 0)
                // were found. The CPU layer only checks declared state at bind
                // time and sees none of it.
                //  if (spDebugController1)
                //      spDebugController1->SetEnableGPUBasedValidation(true);
            }

    }

    void EnableShaderModel()
    {
        const UUID _D3D12ExperimentalShaderModels = { /* 76f5573e-f13a-40f5-b297-81ce9e18933f */
            0x76f5573e,
            0xf13a,
            0x40f5,
            { 0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f }
        };

        //    D3D12EnableExperimentalFeatures(1, &_D3D12ExperimentalShaderModels, nullptr, nullptr);
    }

    void init()
    {
        // Must run before any D3D12 call (EnableGPUDebug() below counts) and
        // before adapter enumeration. Failure is non-fatal.
        nvidia::Streamline::get();

        EnableGPUDebug();
        EnableShaderModel();
    }
}