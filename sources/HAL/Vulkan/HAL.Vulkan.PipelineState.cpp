module HAL:PipelineState;

import vulkan;
import Core;

// Vulkan native implementation of the pipeline on_change() builders and the
// StateObject helpers that the D3D12 build defines in
// D3D12/HAL.D3D12.PipelineState.cpp (also `module HAL:PipelineState`).
// The cache/desc plumbing lives in the common HAL.PipelineState.cpp.
//
// Phase 4 implements VkGraphicsPipeline / VkComputePipeline creation from the
// PipelineStateDesc.  Phase 0: on_change() allocates the tracked-pipeline slot
// so downstream get_tracked()/get_native() are valid, but creates no VkPipeline.

namespace HAL
{
    void PipelineState::on_change()
    {
        tracked_info.reset(new API::TrackedPipeline());
        // Phase 4: translate desc (shaders, blend, raster, RT formats, topology)
        // into VkGraphicsPipelineCreateInfo + vkCreateGraphicsPipelines.
        name = desc.name;
    }

    void ComputePipelineState::on_change()
    {
        tracked_info.reset(new API::TrackedPipeline());
        // Phase 4: vkCreateComputePipelines from desc.shader (SPIR-V).
        name = desc.name;
    }

    void StateObject::on_change()
    {
        // Raytracing / work-graph state objects: VK_KHR_ray_tracing_pipeline,
        // post-MVP.  Phase 0 no-op.
        tracked_info.reset(new API::TrackedPipeline());
        event_change();
    }

    HAL::shader_identifier StateObject::get_shader_id(std::wstring_view /*name*/)
    {
        // Phase (RT): vkGetRayTracingShaderGroupHandlesKHR.
        return {};
    }

    namespace API
    {
        std::string PipelineStateBase::get_cache()
        {
            // Vulkan pipeline cache blobs are handled via VkPipelineCache; for
            // now no serialized cache is produced.
            return "";
        }
    }
}
