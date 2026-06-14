module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:PipelineState;

import vulkan;
import Core;
import :Types;
import :Utils;
import :Device;
import :RootSignature;
import :Shader;
import :PipelineState;
import :API.Device;

// Vulkan Phase 4 — real VkGraphicsPipeline / VkComputePipeline creation.

namespace
{
    // ---- Conversion helpers ------------------------------------------------

    VkPrimitiveTopology to_vk_topology(HAL::PrimitiveTopologyType t,
                                        HAL::PrimitiveTopologyFeed feed)
    {
        using T = HAL::PrimitiveTopologyType;
        using F = HAL::PrimitiveTopologyFeed;
        if (t == T::POINT)    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        if (t == T::LINE)
            return feed == F::STRIP ? VK_PRIMITIVE_TOPOLOGY_LINE_STRIP
                                    : VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        if (t == T::PATCH)    return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        // TRIANGLE (default)
        return feed == F::STRIP ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
                                : VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    VkCullModeFlagBits to_vk_cull(HAL::CullMode c)
    {
        switch (c)
        {
        case HAL::CullMode::None:  return VK_CULL_MODE_NONE;
        case HAL::CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
        default:                   return VK_CULL_MODE_BACK_BIT;
        }
    }

    VkPolygonMode to_vk_fill(HAL::FillMode f)
    {
        return f == HAL::FillMode::Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    }

    VkBlendFactor to_vk_blend(HAL::Blend b)
    {
        switch (b)
        {
        case HAL::Blend::Zero:            return VK_BLEND_FACTOR_ZERO;
        case HAL::Blend::One:             return VK_BLEND_FACTOR_ONE;
        case HAL::Blend::SrcAlpha:        return VK_BLEND_FACTOR_SRC_ALPHA;
        case HAL::Blend::InvSrcAlpha:     return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case HAL::Blend::DstAlpha:        return VK_BLEND_FACTOR_DST_ALPHA;
        case HAL::Blend::InvDstAlpha:     return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case HAL::Blend::SrcColor:        return VK_BLEND_FACTOR_SRC_COLOR;
        case HAL::Blend::InvSrcColor:     return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case HAL::Blend::DestColor:       return VK_BLEND_FACTOR_DST_COLOR;
        case HAL::Blend::InvDestColor:    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case HAL::Blend::SrcAlphaSat:     return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case HAL::Blend::BlendFactor:     return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case HAL::Blend::InvBlendFactor:  return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        default:                          return VK_BLEND_FACTOR_ONE;
        }
    }

    VkCompareOp to_vk_cmp(HAL::ComparisonFunc f)
    {
        switch (f)
        {
        case HAL::ComparisonFunc::NEVER:         return VK_COMPARE_OP_NEVER;
        case HAL::ComparisonFunc::LESS:          return VK_COMPARE_OP_LESS;
        case HAL::ComparisonFunc::EQUAL:         return VK_COMPARE_OP_EQUAL;
        case HAL::ComparisonFunc::LESS_EQUAL:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case HAL::ComparisonFunc::GREATER:       return VK_COMPARE_OP_GREATER;
        case HAL::ComparisonFunc::NOT_EQUAL:     return VK_COMPARE_OP_NOT_EQUAL;
        case HAL::ComparisonFunc::GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        default:                                 return VK_COMPARE_OP_ALWAYS;
        }
    }

    VkStencilOp to_vk_stencil(HAL::StencilOp op)
    {
        switch (op)
        {
        case HAL::StencilOp::Keep:         return VK_STENCIL_OP_KEEP;
        case HAL::StencilOp::Zero:         return VK_STENCIL_OP_ZERO;
        case HAL::StencilOp::Replace:      return VK_STENCIL_OP_REPLACE;
        case HAL::StencilOp::IncrementSat: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case HAL::StencilOp::DecrementSat: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case HAL::StencilOp::Invert:       return VK_STENCIL_OP_INVERT;
        case HAL::StencilOp::Increment:    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        default:                           return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        }
    }

    // Create a VkShaderModule from a SPIR-V blob.
    VkShaderModule make_shader_module(VkDevice dev, const binary& blob)
    {
        if (blob.empty()) return VK_NULL_HANDLE;
        if (blob.size() % 4 != 0) return VK_NULL_HANDLE; // SPIR-V must be 4-byte aligned

        VkShaderModuleCreateInfo ci{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        ci.codeSize = blob.size();
        ci.pCode    = reinterpret_cast<const uint32_t*>(blob.data());

        VkShaderModule module = VK_NULL_HANDLE;
        vkCreateShaderModule(dev, &ci, nullptr, &module);
        return module;
    }
}

namespace HAL::API
{
    TrackedPipeline::~TrackedPipeline()
    {
        if (vk_pipeline != VK_NULL_HANDLE && vk_device != VK_NULL_HANDLE)
            vkDestroyPipeline(vk_device, vk_pipeline, nullptr);
    }
}

namespace HAL
{
    void PipelineState::on_change()
    {
        tracked_info.reset(new API::TrackedPipeline());
        name = desc.name;

        // root_signature is set before on_change() is called (see HAL.PipelineState.cpp).
        if (!root_signature) return;

        auto& api_dev = static_cast<API::Device&>(root_signature->get_device());
        VkDevice vk_dev = api_dev.get_native_device();
        if (vk_dev == VK_NULL_HANDLE) return;

        VkPipelineLayout layout = static_cast<API::RootSignature&>(*root_signature)
                                    .get_vk_pipeline_layout();
        if (layout == VK_NULL_HANDLE) return;

        // ---- Shader modules -------------------------------------------------
        auto make_stage = [&](VkShaderStageFlagBits stage,
                               const binary& blob,
                               const char* entry = "main") -> VkPipelineShaderStageCreateInfo
        {
            VkPipelineShaderStageCreateInfo si{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            si.stage  = stage;
            si.module = make_shader_module(vk_dev, blob);
            si.pName  = entry;
            return si;
        };

        std::vector<VkPipelineShaderStageCreateInfo> stages;
        std::vector<VkShaderModule> modules_to_destroy;
        auto add_stage = [&](VkShaderStageFlagBits stage, auto& shader_ptr)
        {
            if (!shader_ptr) return;
            const auto& blob = shader_ptr->get_blob();
            if (blob.empty()) return;
            // Use the entry point name from compilation (-E flag), e.g. "VS", "PS".
            const auto& ep = shader_ptr->blob.entry_point;
            const char* entry = ep.empty() ? "main" : ep.c_str();
            auto si = make_stage(stage, blob, entry);
            if (si.module != VK_NULL_HANDLE)
            {
                stages.push_back(si);
                modules_to_destroy.push_back(si.module);
            }
        };

        add_stage(VK_SHADER_STAGE_VERTEX_BIT,   desc.vertex);
        add_stage(VK_SHADER_STAGE_FRAGMENT_BIT, desc.pixel);
        add_stage(VK_SHADER_STAGE_GEOMETRY_BIT, desc.geometry);
        add_stage(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,    desc.hull);
        add_stage(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, desc.domain);
        add_stage(VK_SHADER_STAGE_MESH_BIT_EXT, desc.mesh);

        {
            bool has_gs = desc.geometry && !desc.geometry->get_blob().empty();
            Log::get() << "[PSDBG] PSO=" << name.c_str()
                       << " stages=" << stages.size()
                       << " gs_ptr=" << (desc.geometry != nullptr)
                       << " gs_blob=" << (desc.geometry ? (int)desc.geometry->get_blob().size() : -1)
                       << " gs_in_pipeline=" << has_gs << Log::endl;
        }

        if (stages.empty()) return; // no compiled shaders yet

        // ---- Vertex input — vertex-pulling; no VS input attributes ---------
        VkPipelineVertexInputStateCreateInfo vi{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

        // ---- Input assembly -------------------------------------------------
        VkPipelineInputAssemblyStateCreateInfo ia{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        ia.topology = to_vk_topology(desc.topology, PrimitiveTopologyFeed::LIST);

        // ---- Rasterization --------------------------------------------------
        VkPipelineRasterizationStateCreateInfo raster{
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        raster.polygonMode = to_vk_fill(desc.rasterizer.fill_mode);
        raster.cullMode    = to_vk_cull(desc.rasterizer.cull_mode);
        // The negative-height viewport maps NDC Y-up to framebuffer Y-down, identical to
        // D3D12's implicit viewport Y-flip.  Both APIs then compute winding via the same
        // shoelace formula in Y-down space: positive area = CW = front-facing under D3D12's
        // default (FrontCounterClockwise=FALSE).  VK_FRONT_FACE_CLOCKWISE also treats
        // positive area as front-facing, so it is the correct match for D3D12 semantics.
        raster.frontFace   = VK_FRONT_FACE_CLOCKWISE;
        raster.lineWidth   = 1.0f;

        // Conservative rasterization (Phase 5: check extension availability)
        // raster.conservativeRasterizationMode = desc.rasterizer.conservative ?
        //     VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT : VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT;

        // ---- Multisample (1× for now) ---------------------------------------
        VkPipelineMultisampleStateCreateInfo ms{
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // ---- Depth/stencil --------------------------------------------------
        VkPipelineDepthStencilStateCreateInfo ds{
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        ds.depthTestEnable       = desc.rtv.enable_depth ? VK_TRUE : VK_FALSE;
        ds.depthWriteEnable      = desc.rtv.enable_depth_write ? VK_TRUE : VK_FALSE;
        ds.depthCompareOp        = to_vk_cmp(desc.rtv.func);
        ds.stencilTestEnable     = desc.rtv.enable_stencil ? VK_TRUE : VK_FALSE;
        if (ds.stencilTestEnable)
        {
            auto fill_stencil = [](const HAL::StencilDesc& s, uint8_t read_mask,
                                   uint8_t write_mask) -> VkStencilOpState
            {
                VkStencilOpState r{};
                r.failOp      = to_vk_stencil(s.StencilFailOp);
                r.passOp      = to_vk_stencil(s.StencilPassOp);
                r.depthFailOp = to_vk_stencil(s.StencilDepthFailOp);
                r.compareOp   = to_vk_cmp(s.StencilFunc);
                r.compareMask = read_mask;
                r.writeMask   = write_mask;
                return r;
            };
            ds.front = fill_stencil(desc.rtv.stencil_desc,
                                    desc.rtv.stencil_read_mask,
                                    desc.rtv.stencil_write_mask);
            ds.back  = ds.front;
        }

        // ---- Blend ----------------------------------------------------------
        std::vector<VkPipelineColorBlendAttachmentState> blend_attachments;
        const uint32_t rt_count = static_cast<uint32_t>(desc.rtv.rtv_formats.size());
        blend_attachments.reserve(rt_count);
        for (uint32_t ri = 0; ri < rt_count; ++ri)
        {
            const auto& rt_blend = desc.blend.independ_blend
                ? desc.blend.render_target[ri]
                : desc.blend.render_target[0];

            VkPipelineColorBlendAttachmentState att{};
            att.blendEnable         = rt_blend.enabled ? VK_TRUE : VK_FALSE;
            att.srcColorBlendFactor = to_vk_blend(rt_blend.source);
            att.dstColorBlendFactor = to_vk_blend(rt_blend.dest);
            att.colorBlendOp        = VK_BLEND_OP_ADD;
            att.srcAlphaBlendFactor = to_vk_blend(rt_blend.source_alpha);
            att.dstAlphaBlendFactor = to_vk_blend(rt_blend.dest_alpha);
            att.alphaBlendOp        = VK_BLEND_OP_ADD;
            att.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                    | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            blend_attachments.push_back(att);
        }

        VkPipelineColorBlendStateCreateInfo blend_ci{
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        blend_ci.attachmentCount = static_cast<uint32_t>(blend_attachments.size());
        blend_ci.pAttachments    = blend_attachments.data();

        // ---- Viewport (dynamic) --------------------------------------------
        VkPipelineViewportStateCreateInfo viewport{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewport.viewportCount = 1;
        viewport.scissorCount  = 1;

        // ---- Dynamic state --------------------------------------------------
        const VkDynamicState dyn_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE,
            // Topology is set per-draw via set_topology() (mirrors D3D12's
            // IASetPrimitiveTopology: PSO has topology TYPE, draw call sets LIST/STRIP).
            // Promoted to Vulkan 1.3 core via VK_EXT_extended_dynamic_state.
            VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
        };
        VkPipelineDynamicStateCreateInfo dyn_ci{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dyn_ci.dynamicStateCount = 4;
        dyn_ci.pDynamicStates    = dyn_states;

        // ---- Dynamic rendering attachment formats ---------------------------
        std::vector<VkFormat> color_formats;
        color_formats.reserve(rt_count);
        for (auto fmt : desc.rtv.rtv_formats)
            color_formats.push_back(to_native(fmt));

        VkPipelineRenderingCreateInfoKHR rendering_ci{
            VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
        rendering_ci.colorAttachmentCount    = static_cast<uint32_t>(color_formats.size());
        rendering_ci.pColorAttachmentFormats = color_formats.data();
        rendering_ci.depthAttachmentFormat   = desc.rtv.enable_depth
            ? to_native(desc.rtv.ds_format) : VK_FORMAT_UNDEFINED;
        rendering_ci.stencilAttachmentFormat = (desc.rtv.enable_stencil && desc.rtv.enable_depth)
            ? to_native(desc.rtv.ds_format) : VK_FORMAT_UNDEFINED;

        // ---- Tessellation state (required when hull/domain shaders are present) ---
        VkPipelineTessellationStateCreateInfo tess_ci{
            VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
        // Default to 3 control points (triangle patches).  The actual runtime
        // value is supplied via set_topology(); for now most tessellated meshes
        // use triangle patches.  TODO: store patch_control_points in PSO desc.
        tess_ci.patchControlPoints = 3;
        const bool has_tessellation = (desc.hull != nullptr) || (desc.domain != nullptr);

        // ---- Graphics pipeline ---------------------------------------------
        VkGraphicsPipelineCreateInfo gp_ci{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        gp_ci.stageCount          = static_cast<uint32_t>(stages.size());
        gp_ci.pStages             = stages.data();
        gp_ci.pVertexInputState   = &vi;
        gp_ci.pInputAssemblyState = &ia;
        gp_ci.pTessellationState  = has_tessellation ? &tess_ci : nullptr;
        gp_ci.pRasterizationState = &raster;
        gp_ci.pMultisampleState   = &ms;
        gp_ci.pDepthStencilState  = &ds;
        gp_ci.pColorBlendState    = &blend_ci;
        gp_ci.pViewportState      = &viewport;
        gp_ci.pDynamicState       = &dyn_ci;
        gp_ci.layout              = layout;
        gp_ci.pNext               = &rendering_ci; // dynamic rendering (no render pass)

        VkPipeline pipeline = VK_NULL_HANDLE;
        VkResult r = vkCreateGraphicsPipelines(vk_dev, VK_NULL_HANDLE, 1, &gp_ci,
                                               nullptr, &pipeline);
        if (r == VK_SUCCESS)
        {
            tracked_info->vk_pipeline = pipeline;
            tracked_info->vk_device   = vk_dev;
            tracked_info->vk_topology = ia.topology;
        }
        else
            Log::get() << Log::LEVEL_WARNING << "vkCreateGraphicsPipelines failed "
                       << name.c_str() << " code=" << static_cast<int>(r) << Log::endl;

        // Shader modules can be destroyed after pipeline creation.
        for (auto m : modules_to_destroy)
            if (m != VK_NULL_HANDLE)
                vkDestroyShaderModule(vk_dev, m, nullptr);
    }

    void ComputePipelineState::on_change()
    {
        tracked_info.reset(new API::TrackedPipeline());
        name = desc.name;

        if (!root_signature || !desc.shader) return;

        auto& api_dev = static_cast<API::Device&>(root_signature->get_device());
        VkDevice vk_dev = api_dev.get_native_device();
        if (vk_dev == VK_NULL_HANDLE) return;

        VkPipelineLayout layout = static_cast<API::RootSignature&>(*root_signature)
                                    .get_vk_pipeline_layout();
        if (layout == VK_NULL_HANDLE) return;

        const auto& blob = desc.shader->get_blob();
        if (blob.empty() || blob.size() % 4 != 0) return;

        // Use the entry point name from compilation (-E flag), e.g. "CS".
        const auto& ep = desc.shader->blob.entry_point;
        std::string cs_entry = ep.empty() ? "main" : ep;

        VkShaderModuleCreateInfo module_ci{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        module_ci.codeSize = blob.size();
        module_ci.pCode    = reinterpret_cast<const uint32_t*>(blob.data());

        VkShaderModule cs_module = VK_NULL_HANDLE;
        vkCreateShaderModule(vk_dev, &module_ci, nullptr, &cs_module);
        if (cs_module == VK_NULL_HANDLE) return;

        VkComputePipelineCreateInfo cp_ci{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        cp_ci.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cp_ci.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
        cp_ci.stage.module = cs_module;
        cp_ci.stage.pName  = cs_entry.c_str();
        cp_ci.layout       = layout;

        VkPipeline pipeline = VK_NULL_HANDLE;
        vkCreateComputePipelines(vk_dev, VK_NULL_HANDLE, 1, &cp_ci, nullptr, &pipeline);
        tracked_info->vk_pipeline = pipeline;
        tracked_info->vk_device   = vk_dev;
        tracked_info->is_compute  = true;

        vkDestroyShaderModule(vk_dev, cs_module, nullptr);
    }

    void StateObject::on_change()
    {
        tracked_info.reset(new API::TrackedPipeline());
        event_change();
    }

    HAL::shader_identifier StateObject::get_shader_id(std::wstring_view)
    {
        return {};
    }

    namespace API
    {
        std::string PipelineStateBase::get_cache()
        {
            return "";
        }
    }
}
