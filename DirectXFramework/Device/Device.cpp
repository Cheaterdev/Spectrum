#include "pch.h"

namespace DX11
{
    Device::~Device()
    {
        swap_chains.clear();
        immediate_context->get_native()->ClearState();
        immediate_context->get_native()->Flush();
        immediate_context = nullptr;
        gi_factory = nullptr;
        gi_adapter = nullptr;
        gi_device = nullptr;

        if (debug)
            debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

        debug = nullptr;
        unsigned int references = device.p->Release();
        device.p = nullptr;

        if (references > 0)
            Log::get() << " some data is not released: " << references << Log::endl;
    }

    Device::Device()
    {
        UINT flags = 0;
#ifdef DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        DX11_DeviceContext native_context;
        std::vector<D3D_FEATURE_LEVEL> feature_requested = { /*D3D_FEATURE_LEVEL_11_3, D3D_FEATURE_LEVEL_11_2,*/ D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
        D3D_FEATURE_LEVEL FeatureLevelsSupported;
        D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, feature_requested.data(), static_cast<UINT>(feature_requested.size()), D3D11_SDK_VERSION, &device,
                          &FeatureLevelsSupported,
                          &native_context);
        immediate_context.reset(new Context(native_context));
        /*
        D3D11_FEATURE_DATA_D3D11_OPTIONS1 featureData)*999
        HRESULT hr = device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS1, &featureData, sizeof(featureData));
        */
        device->QueryInterface(__uuidof(DXGI_Device), (void**)&gi_device);
        gi_device->GetParent(__uuidof(DXGI_Adapter), (void**)&gi_adapter);
        gi_adapter->GetParent(__uuidof(DXGI_Factory), (void**)&gi_factory);
        gi_device->QueryInterface(IID_ID3D11Debug, (void**)&debug);

        if (debug)
            debug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_queue);

        unsigned int i = 0;
        DXGI_Output output = nullptr;

        while (gi_adapter->EnumOutputs(i++, (IDXGIOutput**)&output) != DXGI_ERROR_NOT_FOUND)
        {
            outputs.push_back(output);
            DXGI_OUTPUT_DESC desc;
            output->GetDesc(&desc);
            Logger::get() << "Output found " << desc.DeviceName << Log::endl;
            DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
            unsigned int num = 0;
            output->GetDisplayModeList(format, flags, &num, 0);
            std::vector<DXGI_MODE_DESC> mode_descs(num);
            output->GetDisplayModeList(format, flags, &num, mode_descs.data());

            for (auto desc : mode_descs)
                Logger::get() << "---- Mode found " << desc.Width << "x" << desc.Height << " " << desc.RefreshRate.Numerator / desc.RefreshRate.Denominator << "Hz" << Log::endl;

            output = nullptr;
        }
    }


}
