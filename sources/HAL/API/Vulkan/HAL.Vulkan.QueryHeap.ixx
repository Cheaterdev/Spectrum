export module HAL:API.QueryHeap;
import vulkan;
import Core;

import :Types;
import :Sampler;
import :Utils;
import :API.Device;

export namespace HAL
{
    namespace API
    {
        class QueryHeap
        {
        protected:
            VkQueryPool vk_query_pool = VK_NULL_HANDLE;
            VkDevice    vk_qh_device  = VK_NULL_HANDLE; // for destructor
        public:
            VkQueryPool get_native() const { return vk_query_pool; }
            virtual ~QueryHeap();
        };
    }
}
