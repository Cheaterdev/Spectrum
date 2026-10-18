module RenderSystem;

import HAL;
import Core;

std::shared_ptr<HAL::Device> RenderSystem::select_adapter()
{
    HAL::init();

    HAL::Adapter::ptr result, any_result;
    std::string result_name;
    HAL::Adapters::get().enumerate([&](HAL::Adapter::ptr adapter) {
        const auto desc = adapter->get_info();
        Log::get() << "adapter: " << desc.name << Log::endl;

        const auto props = HAL::Device::probe(adapter);

        // SPECTRUM_ADAPTER=<part of the name> (e.g. AMD) restricts selection to
        // matching adapters -- to run on a second GPU of the same machine.
        if (const char* wanted = std::getenv("SPECTRUM_ADAPTER"); wanted && *wanted)
        {
            const std::wstring wanted_w(wanted, wanted + std::strlen(wanted));
            if (!desc.name.contains(wanted_w))
                return;
        }


        if (!result && props.mesh_shader && props.full_bindless &&
			!desc.name.contains(L"Basic"))
        {
            result = adapter;
            result_name = props.name;
        }
        else if (props.full_bindless && !any_result)
        {
            any_result = adapter;
        }
    });

    if (!result)
    {
        result = any_result;
        if (result)
            result_name = HAL::Device::probe(result).name;
    }
    if (!result) return nullptr;

    Log::get() << "Selected adapter: " << result_name << Log::endl;

    {
        const auto props = HAL::Device::probe(result);
        Log::get() << (std::string("DLSS: super-resolution=") + (props.dlss ? "yes" : "no")
                       + " ray-reconstruction=" + (props.dlss_rr ? "yes" : "no")) << Log::endl;
    }

    HAL::DeviceDesc dev_desc;
    dev_desc.adapter = result;
    return std::make_shared<HAL::Device>(dev_desc);
}

std::shared_ptr<RenderSystem> RenderSystem::create_singleton()
{
    auto device = select_adapter();
    if (!device) return nullptr;

    device->init_managers();

    return std::make_shared<RenderSystem>(std::move(device));
}
