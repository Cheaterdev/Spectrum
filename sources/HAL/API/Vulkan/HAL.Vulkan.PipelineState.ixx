export module HAL:API.PipelineState;
import vulkan;
import Core;
import :Types;
import :Utils;

export namespace HAL
{
    namespace API
    {
        class TrackedPipeline : public TrackedObject
        {
        public:
            VkPipeline           vk_pipeline  = VK_NULL_HANDLE;
            VkDevice             vk_device    = VK_NULL_HANDLE; // for destructor cleanup
            bool                 is_compute   = false;
            bool                 is_ray_tracing = false;
            bool                 is_mesh      = false;   // task/mesh pipeline (no vertex input / topology)
            VkPrimitiveTopology  vk_topology  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            bool is_built() const { return vk_pipeline != VK_NULL_HANDLE; }

            virtual ~TrackedPipeline();
        };

        class PipelineStateBase
        {
        public:
            virtual ~PipelineStateBase() = default;
            std::string get_cache();
        };

        // A D3D12 ray tracing state object. A Collection is only a description;
        // the Raytracing object flattens its own and its collections' libraries
        // and hit groups into one VkRayTracingPipeline and keeps every group's
        // handle, which get_shader_id() looks up by export name.
        class StateObject
        {
        protected:
            std::map<std::wstring, uint32_t> group_indices;
            std::vector<std::byte>           group_handles;   // shader_identifier-sized, per group
        };
    }
}
