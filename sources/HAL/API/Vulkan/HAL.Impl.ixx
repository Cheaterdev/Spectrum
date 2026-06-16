export module HAL:Impl;
import vulkan;
import :Debug;

export namespace HAL
{
    void EnableGPUDebug();   // enables Vulkan validation layers
    void EnableShaderModel(); // no-op in Vulkan (shader model is SPIR-V)
    void init();

    // The VkInstance is created once in HAL::init() and lives for the duration
    // of the program.  Both Adapters (for enumeration) and Device (for logical
    // device creation) obtain it here rather than creating their own instances.
    VkInstance get_vk_instance();
    VkDebugUtilsMessengerEXT get_vk_debug_messenger();
}
