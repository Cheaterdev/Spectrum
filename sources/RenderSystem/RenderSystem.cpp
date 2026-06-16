module RenderSystem;

import HAL;
import Core;

std::shared_ptr<HAL::Device> RenderSystem::select_adapter()
{
    HAL::init();

    std::shared_ptr<HAL::Device> result, any_result;
    HAL::Adapters::get().enumerate([&](HAL::Adapter::ptr adapter) {
        const auto& desc = adapter->get_desc();
        Log::get() << "adapter: " << desc.Description << Log::endl;

        HAL::DeviceDesc dev_desc;
        dev_desc.adapter = adapter;
        auto device = std::make_shared<HAL::Device>(dev_desc);
        const auto& props = device->get_properties();

        if (!result && props.mesh_shader && props.full_bindless &&
            std::wstring(desc.Description).find(L"Basic") == std::wstring::npos)
            result = device;
        else if (props.full_bindless)
            any_result = device;
    });

    if (!result) result = any_result;

    if (result)
        Log::get() << "Selected device: " << result->get_properties().name << Log::endl;

    return result;
}

std::shared_ptr<RenderSystem> RenderSystem::create_singleton()
{
    auto device = select_adapter();
    if (!device) return nullptr;

    device->init_managers();
    return std::make_shared<RenderSystem>(std::move(device));
}
