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
            VkPipeline vk_pipeline = VK_NULL_HANDLE;
        };

        class PipelineStateBase
        {
        public:
            virtual ~PipelineStateBase() = default;
            std::string get_cache();
        };

        // StateObject: D3D12 work-graph / raytracing state object.
        // Not supported on Vulkan for Phase 0; stub kept for interface compat.
        class StateObject
        {
        public:
            D3D12_PROGRAM_IDENTIFIER id{};
            uint64 buffer_size = 0;
        };
    }
}
