module HAL:Impl;
import :Adapter;

namespace HAL
{
    void EnableGPUDebug()
    {
        // Validation layers are enabled in Device::init() when debug mode is active.
    }

    void EnableShaderModel()
    {
        // No-op: SPIR-V shaders have no "shader model" negotiation.
    }

    void init()
    {
        // Initialise singleton Adapters — actual VkInstance lives on Device.
        HAL::Adapters::create();
    }
}
