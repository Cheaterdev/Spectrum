module HAL:PipelineState;

import stl.core;
import vulkan;
import Core;
#include "HAL.Vulkan.SpirvEntryPoints.h"
import :Types;
import :Utils;
import :Device;
import :RootSignature;
import :Shader;
import :ShaderCompiler;   // ShaderCompiler::can_compile
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

        // The slots every stage reads (SPIR-V reflection, HAL.Vulkan.
        // ShaderReflection.cpp). commit_tables records resource usage -- and so
        // the barrier compiler emits barriers -- only for these.
        slots.clear();
        if (desc.vertex)       slots.merge(desc.vertex->slots_usage);
        if (desc.pixel)         slots.merge(desc.pixel->slots_usage);
        if (desc.geometry)      slots.merge(desc.geometry->slots_usage);
        if (desc.hull)          slots.merge(desc.hull->slots_usage);
        if (desc.domain)        slots.merge(desc.domain->slots_usage);
        if (desc.amplification) slots.merge(desc.amplification->slots_usage);
        if (desc.mesh)          slots.merge(desc.mesh->slots_usage);

        // root_signature is set before on_change() is called (see HAL.PipelineState.cpp).
        if (!root_signature) return;

        auto& api_dev = static_cast<API::Device&>(root_signature->get_device());
        VkDevice vk_dev = api_dev.get_native_device();
        if (vk_dev == VK_NULL_HANDLE) return;

        // Deserialized material pipelines (HAL::Wrapper) still describe the mesh-shader form on a GPU without
        // it, where the driver rejects them (VK_ERROR_UNKNOWN); those materials draw through their vertex-stage pipelines.
        if ((desc.mesh || desc.amplification) && !root_signature->get_device().is_mesh_shader_supported())
            return;

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
        // A declared stage whose shader is still compiling (material shaders are
        // async) must hold the whole pipeline back: dropping just that stage builds
        // a valid pipeline without it -- a missing pixel shader then draws depth
        // and no color, black until the shader lands and on_change() runs again.
        bool all_stages_ready = true;
        auto add_stage = [&](VkShaderStageFlagBits stage, auto& shader_ptr)
        {
            if (!shader_ptr) return;
            const auto& blob = shader_ptr->get_blob();
            if (blob.empty()) { all_stages_ready = false; return; }
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
        // Mesh pipeline: amplification shader → task stage, mesh shader → mesh stage.
        add_stage(VK_SHADER_STAGE_TASK_BIT_EXT, desc.amplification);
        add_stage(VK_SHADER_STAGE_MESH_BIT_EXT, desc.mesh);

        if (stages.empty() || !all_stages_ready)
        {
            // With no shader compiler an empty shader never fills in later (with one it does: material shaders are
            // async), and every draw with this pipeline is then skipped without a word.
            if (!ShaderCompiler::can_compile)
                Log::get() << Log::LEVEL_ERROR << "pipeline '" << desc.name << "' not built: a shader has no blob "
                           << "(precompiled shader missing from the cache)" << Log::endl;
            for (VkShaderModule m : modules_to_destroy)
                vkDestroyShaderModule(vk_dev, m, nullptr);
            return; // vk_pipeline stays null; draws with it are skipped until the shaders land
        }

        // A mesh pipeline has no vertex-pulling input assembly and must NOT declare
        // the primitive-topology dynamic state (VUID-...-07065).
        const bool has_mesh = (desc.mesh != nullptr);

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

        // D3D12 DepthBias / DepthBiasClamp / SlopeScaledDepthBias.
        raster.depthBiasEnable         = (desc.rasterizer.depth_bias != 0 || desc.rasterizer.slope_scaled_depth_bias != 0.0f) ? VK_TRUE : VK_FALSE;
        raster.depthBiasConstantFactor = float(desc.rasterizer.depth_bias);
        raster.depthBiasClamp          = desc.rasterizer.depth_bias_clamp;
        raster.depthBiasSlopeFactor    = desc.rasterizer.slope_scaled_depth_bias;

        VkPipelineRasterizationConservativeStateCreateInfoEXT conservative{
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT };
        if (desc.rasterizer.conservative)
        {
            ASSERT(api_dev.supports_conservative_raster() && "conservative PSO on a device without VK_EXT_conservative_rasterization");
            if (api_dev.supports_conservative_raster())
            {
                conservative.conservativeRasterizationMode = VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
                raster.pNext = &conservative;
            }
        }

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
        std::vector<VkDynamicState> dyn_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_STENCIL_REFERENCE,
        };
        // Topology is set per-draw via set_topology() (mirrors D3D12's
        // IASetPrimitiveTopology: PSO has topology TYPE, draw sets LIST/STRIP).
        // Mesh pipelines have no input assembly, so this dynamic state is illegal there.
        if (!has_mesh)
            dyn_states.push_back(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);

        VkPipelineDynamicStateCreateInfo dyn_ci{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dyn_ci.dynamicStateCount = static_cast<uint32_t>(dyn_states.size());
        dyn_ci.pDynamicStates    = dyn_states.data();

        // ---- Dynamic rendering attachment formats ---------------------------
        std::vector<VkFormat> color_formats;
        color_formats.reserve(rt_count);
        for (auto fmt : desc.rtv.rtv_formats)
            color_formats.push_back(to_native(fmt));

        VkPipelineRenderingCreateInfoKHR rendering_ci{
            VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
        rendering_ci.colorAttachmentCount    = static_cast<uint32_t>(color_formats.size());
        rendering_ci.pColorAttachmentFormats = color_formats.data();
        // Like D3D12's DSVFormat: the bound depth buffer's format whether or not
        // this PSO tests against it -- a depth attachment bound with test off must
        // still match the pipeline's format.
        const VkFormat ds_native = desc.rtv.ds_format != Format::UNKNOWN ? to_native(desc.rtv.ds_format) : VK_FORMAT_UNDEFINED;
        rendering_ci.depthAttachmentFormat   = ds_native;
        rendering_ci.stencilAttachmentFormat = (desc.rtv.enable_stencil && desc.rtv.enable_depth) ? ds_native : VK_FORMAT_UNDEFINED;

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
        // Mesh pipelines have no vertex-pulling input assembly (ignored / must be null).
        gp_ci.pVertexInputState   = has_mesh ? nullptr : &vi;
        gp_ci.pInputAssemblyState = has_mesh ? nullptr : &ia;
        gp_ci.pTessellationState  = has_tessellation ? &tess_ci : nullptr;
        gp_ci.pRasterizationState = &raster;
        gp_ci.pMultisampleState   = &ms;
        gp_ci.pDepthStencilState  = &ds;
        gp_ci.pColorBlendState    = &blend_ci;
        gp_ci.pViewportState      = &viewport;
        gp_ci.pDynamicState       = &dyn_ci;
        gp_ci.layout              = api_dev.get_pipeline_layout();
        gp_ci.pNext               = &rendering_ci;

        VkPipeline pipeline = VK_NULL_HANDLE;
        VkResult r = vkCreateGraphicsPipelines(vk_dev, VK_NULL_HANDLE, 1, &gp_ci,
                                               nullptr, &pipeline);
        if (r == VK_SUCCESS)
        {
            tracked_info->vk_pipeline = pipeline;
            tracked_info->vk_device   = vk_dev;
            tracked_info->vk_topology = ia.topology;
            tracked_info->is_mesh     = has_mesh;
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
        tracked_info->is_compute = true;   // also while not built: binding it then clears the compute slot
        name = desc.name;

        slots.clear();
        if (desc.shader)
            slots.merge(desc.shader->slots_usage);

        if (!root_signature || !desc.shader) return;

        auto& api_dev = static_cast<API::Device&>(root_signature->get_device());
        VkDevice vk_dev = api_dev.get_native_device();
        if (vk_dev == VK_NULL_HANDLE) return;

        const auto& blob = desc.shader->get_blob();
        if (blob.empty() || blob.size() % 4 != 0)
        {
            Log::get() << Log::LEVEL_ERROR << "compute pipeline '" << desc.name << "': no usable shader blob (" << blob.size() << " bytes)" << Log::endl;
            return;
        }

        // Use the entry point name from compilation (-E flag), e.g. "CS".
        const auto& ep = desc.shader->blob.entry_point;
        std::string cs_entry = ep.empty() ? "main" : ep;

        VkShaderModuleCreateInfo module_ci{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        module_ci.codeSize = blob.size();
        module_ci.pCode    = reinterpret_cast<const uint32_t*>(blob.data());

        VkShaderModule cs_module = VK_NULL_HANDLE;
        const VkResult module_result = vkCreateShaderModule(vk_dev, &module_ci, nullptr, &cs_module);
        if (cs_module == VK_NULL_HANDLE)
        {
            Log::get() << Log::LEVEL_ERROR << "vkCreateShaderModule failed for compute pipeline '" << desc.name << "' code=" << (int)module_result << Log::endl;
            return;
        }

        VkComputePipelineCreateInfo cp_ci{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        cp_ci.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cp_ci.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
        cp_ci.stage.module = cs_module;
        cp_ci.stage.pName  = cs_entry.c_str();
        cp_ci.layout       = api_dev.get_pipeline_layout();

        VkPipeline pipeline = VK_NULL_HANDLE;
        const VkResult pipeline_result = vkCreateComputePipelines(vk_dev, VK_NULL_HANDLE, 1, &cp_ci, nullptr, &pipeline);
        if (pipeline_result != VK_SUCCESS)
            Log::get() << Log::LEVEL_ERROR << "vkCreateComputePipelines failed for '" << desc.name << "' code=" << (int)pipeline_result << Log::endl;
        tracked_info->vk_pipeline = pipeline;
        tracked_info->vk_device   = vk_dev;

        vkDestroyShaderModule(vk_dev, cs_module, nullptr);
    }

    void StateObject::on_change()
    {
        tracked_info.reset(new API::TrackedPipeline());
        tracked_info->is_ray_tracing = true;
        group_indices.clear();
        group_handles.clear();

        // What every raytrace dispatch touches through its shaders, for the
        // barrier tracker -- same merge as the D3D12 state object.
        slots.clear();
        for (auto& l : desc.libraries)
            if (l.library) slots.merge(l.library->slots_usage);
        for (auto& c : desc.collections)
            slots.merge(c->slots);

        // A collection only describes; the pipeline is built where it is linked.
        if (desc.type == StateObjectType::Collection || !desc.global_root)
        {
            event_change();
            return;
        }

        auto& dev = static_cast<API::Device&>(desc.global_root->get_device());
        const VkDevice vk_dev = dev.get_native_device();
        if (!dev.fn_create_rt_pipelines || vk_dev == VK_NULL_HANDLE)
        {
            event_change();
            return;
        }

        std::vector<const StateObjectDesc*> parts{ &desc };
        for (auto& c : desc.collections)
            parts.push_back(&c->desc);

        std::vector<VkPipelineShaderStageCreateInfo>     stages;
        std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
        std::map<const void*, VkShaderModule>            modules;   // by library blob
        std::map<std::wstring, uint32_t>                 stage_of_export;
        std::list<std::string>                           entry_names;   // stable storage for pName
        std::vector<std::wstring>                        group_names;
        UINT recursion = 1;

        auto add_group = [&](std::wstring name, VkRayTracingShaderGroupTypeKHR type, uint32_t general, uint32_t closest, uint32_t any) {
            VkRayTracingShaderGroupCreateInfoKHR g{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
            g.type               = type;
            g.generalShader      = general;
            g.closestHitShader   = closest;
            g.anyHitShader       = any;
            g.intersectionShader = VK_SHADER_UNUSED_KHR;
            groups.push_back(g);
            group_names.push_back(std::move(name));
        };

        for (const StateObjectDesc* part : parts)
        {
            recursion = std::max(recursion, part->MaxTraceRecursionDepth);

            for (auto& l : part->libraries)
            {
                if (!l.library) continue;
                const auto& blob = l.library->get_blob();
                const auto  eps  = Spirv::entry_points(blob.data(), blob.size());
                if (eps.empty()) continue;   // not compiled (yet): its exports stay unresolved

                VkShaderModule& shader_module = modules[blob.data()];
                if (shader_module == VK_NULL_HANDLE)
                    shader_module = make_shader_module(vk_dev, blob);
                if (shader_module == VK_NULL_HANDLE) continue;

                // exports: exported name -> library function it renames (empty: same).
                // A library with no explicit exports exports every entry point.
                std::map<std::wstring, std::wstring> exports = l.exports;
                if (exports.empty())
                    for (auto& ep : eps)
                        exports[convert(ep.name)] = L"";

                for (auto& [exported, was] : exports)
                {
                    const std::string fn = convert(was.empty() ? exported : was);
                    auto ep = std::find_if(eps.begin(), eps.end(), [&](const Spirv::EntryPoint& e) { return e.name == fn; });
                    if (ep == eps.end()) continue;

                    VkShaderStageFlagBits stage;
                    switch (ep->execution_model)
                    {
                    case Spirv::RayGeneration: stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;      break;
                    case Spirv::Miss:          stage = VK_SHADER_STAGE_MISS_BIT_KHR;        break;
                    case Spirv::ClosestHit:    stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; break;
                    case Spirv::AnyHit:        stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;     break;
                    default: continue;   // intersection/callable: nothing uses them yet
                    }

                    VkPipelineShaderStageCreateInfo si{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
                    si.stage  = stage;
                    si.module = shader_module;
                    si.pName  = entry_names.emplace_back(fn).c_str();
                    const uint32_t index = static_cast<uint32_t>(stages.size());
                    stages.push_back(si);
                    stage_of_export[exported] = index;

                    // Raygen and miss shaders are their own (general) groups.
                    if (stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR || stage == VK_SHADER_STAGE_MISS_BIT_KHR)
                        add_group(exported, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, index, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR);
                }
            }
        }

        for (const StateObjectDesc* part : parts)
            for (auto& hg : part->hit_groups)
            {
                auto stage_index = [&](const std::wstring& name) {
                    auto it = name.empty() ? stage_of_export.end() : stage_of_export.find(name);
                    return it == stage_of_export.end() ? VK_SHADER_UNUSED_KHR : it->second;
                };
                const uint32_t closest = stage_index(hg.closest_hit_shader);
                const uint32_t any     = stage_index(hg.any_hit_shader);
                if (closest == VK_SHADER_UNUSED_KHR && any == VK_SHADER_UNUSED_KHR)
                    continue;   // its library isn't compiled yet
                add_group(hg.name, VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, VK_SHADER_UNUSED_KHR, closest, any);
            }

        VkPipeline pipeline = VK_NULL_HANDLE;
        if (!groups.empty())
        {
            VkRayTracingPipelineCreateInfoKHR ci{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
            ci.stageCount                   = static_cast<uint32_t>(stages.size());
            ci.pStages                      = stages.data();
            ci.groupCount                   = static_cast<uint32_t>(groups.size());
            ci.pGroups                      = groups.data();
            ci.maxPipelineRayRecursionDepth = std::min<uint32_t>(recursion, std::max<uint32_t>(1, dev.rt_max_recursion_depth));
            ci.layout                       = dev.get_pipeline_layout();
            dev.fn_create_rt_pipelines(vk_dev, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &ci, nullptr, &pipeline);
            ASSERT(pipeline != VK_NULL_HANDLE && "vkCreateRayTracingPipelinesKHR failed");
        }

        for (auto& [blob, shader_module] : modules)
            vkDestroyShaderModule(vk_dev, shader_module, nullptr);

        if (pipeline != VK_NULL_HANDLE)
        {
            tracked_info->vk_pipeline = pipeline;
            tracked_info->vk_device   = vk_dev;

            const size_t handle_size = sizeof(HAL::shader_identifier);
            group_handles.resize(groups.size() * handle_size);
            dev.fn_get_rt_group_handles(vk_dev, pipeline, 0, static_cast<uint32_t>(groups.size()),
                                        group_handles.size(), group_handles.data());
            for (uint32_t i = 0; i < group_names.size(); ++i)
                group_indices[group_names[i]] = i;
        }

        event_change();
    }

    HAL::shader_identifier StateObject::get_shader_id(std::wstring_view name)
    {
        // An all-zero handle is a null shader group: a ray reaching it does nothing.
        HAL::shader_identifier id{};
        auto it = group_indices.find(std::wstring(name));
        if (it != group_indices.end())
            std::memcpy(id.data(), group_handles.data() + size_t(it->second) * id.size(), id.size());
        return id;
    }

    namespace API
    {
        std::string PipelineStateBase::get_cache()
        {
            return "";
        }
    }
}
