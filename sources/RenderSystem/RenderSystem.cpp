module RenderSystem;

import HAL;
import Core;

std::shared_ptr<HAL::Device> RenderSystem::select_adapter()
{
    HAL::init();

    HAL::Adapter::ptr result, any_result;
    std::string result_name;
    HAL::Adapters::get().enumerate([&](HAL::Adapter::ptr adapter) {
        const auto& desc = adapter->get_desc();
        Log::get() << "adapter: " << desc.Description << Log::endl;

        const auto props = HAL::Device::probe(adapter);

        //if (std::wstring(desc.Description).find(L"AMD") == std::wstring::npos) return;


        if (!result && props.mesh_shader && props.full_bindless &&
            std::wstring(desc.Description).find(L"Basic") == std::wstring::npos)
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
