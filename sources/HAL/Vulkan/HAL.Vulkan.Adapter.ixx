export module HAL:Adapter;
import :Utils;   // pulls in DXGI_ADAPTER_DESC stub

import vulkan;
import Core;

export namespace HAL
{
    struct AdapterDesc {};   // empty common struct kept for compat

    class Adapter
    {
    public:
        VkPhysicalDevice vk_physical = VK_NULL_HANDLE;
        DXGI_ADAPTER_DESC adapter_desc{};

        explicit Adapter(VkPhysicalDevice physical);

    public:
        using ptr = std::shared_ptr<Adapter>;

        const DXGI_ADAPTER_DESC& get_desc() const;

    private:
        friend class Adapters;
    };

    class Adapters : public Singleton<Adapters>
    {
        VkInstance vk_instance = VK_NULL_HANDLE;  // borrowed from Device

        friend class Singleton<Adapters>;
        Adapters() = default;
    public:
        // Called by Device::init() after instance creation.
        void set_instance(VkInstance instance) { vk_instance = instance; }

        void enumerate(auto f)
        {
            if (vk_instance == VK_NULL_HANDLE) return;

            uint32_t count = 0;
            vkEnumeratePhysicalDevices(vk_instance, &count, nullptr);
            std::vector<VkPhysicalDevice> devices(count);
            vkEnumeratePhysicalDevices(vk_instance, &count, devices.data());

            for (auto pd : devices)
                f(std::make_shared<Adapter>(pd));
        }
    };
}
