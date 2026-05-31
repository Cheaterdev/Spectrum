export module HAL:Impl;
import vulkan;
import :Debug;

export namespace HAL
{
    void EnableGPUDebug();   // enables Vulkan validation layers
    void EnableShaderModel(); // no-op in Vulkan (shader model is SPIR-V)
    void init();
}
