export module HAL:Adapter;
import :Utils;   // pulls in DXGI_ADAPTER_DESC stub
import :Impl;    // get_vk_instance()

import vulkan;
import Core;

export namespace HAL
{
    struct AdapterDesc {};   // empty common struct kept for compat

    class Adapter
    {
        friend class Adapters;

    protected:
        VkPhysicalDevice  vk_physical  = VK_NULL_HANDLE;
        DXGI_ADAPTER_DESC adapter_desc = {};

    public:
        using ptr = std::shared_ptr<Adapter>;

        explicit Adapter(VkPhysicalDevice physical);
        const DXGI_ADAPTER_DESC& get_desc() const;

        // Public accessor — used by API::Device::init (cannot friend across
        // the circular Adapter ↔ API.Device import dependency).
        VkPhysicalDevice get_vk_physical() const { return vk_physical; }
    };

    class Adapters : public Singleton<Adapters>
    {
        friend class Singleton<Adapters>;
        Adapters() = default;
    public:
        void enumerate(auto f)
        {
            // Instance was created in HAL::init() before this is called.
            VkInstance instance = HAL::get_vk_instance();
            if (instance == VK_NULL_HANDLE) return;

            uint32_t count = 0;
            vkEnumeratePhysicalDevices(instance, &count, nullptr);
            std::vector<VkPhysicalDevice> devices(count);
            vkEnumeratePhysicalDevices(instance, &count, devices.data());

            for (auto pd : devices)
                f(std::make_shared<Adapter>(pd));
        }
    };
}
