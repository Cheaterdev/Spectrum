export module Graphics:System;

import RenderSystem;
import :RTX;
import :AssetRenderer;
import :TextureAsset;
import :Asset;
import :Materials.UniversalMaterial;
import :MeshAsset;
import GUI;
import Core;
import TextSystem;

// Manages the lifecycle of all graphics singletons in the correct order.
// Calls RenderSystem::create() internally to create the device.
// Returns nullptr from create() if no suitable GPU is found.
export class GraphicsSystem : public Singleton<GraphicsSystem>
{
    friend class Singleton<GraphicsSystem>;

public:
    static std::shared_ptr<GraphicsSystem> create_singleton()
    {
        if (!RenderSystem::create()) return nullptr;

        if (RenderSystem::get().device().is_rtx_supported())
            RTX::create();
#ifndef HAL_BACKEND_VULKAN
        AssetRenderer::create();
#endif
        AssetManager::create();

        // Force-load engine assets (brdf, best_fit_normals, sky/SMAA LUTs, etc.)
        // now, while the device and AssetManager are up but before any frame
        // renders. Left lazy, an engine asset first used inside a render pass is
        // deserialized mid-frame in COMMON and consumed before its deferred
        // promote runs -> D3D12 #1334. Centralised here so it is not any single
        // pass's responsibility (brdf, for one, is used by both PSSM and the
        // AssetRenderer).
        preload_engine_assets();

        return std::make_shared<GraphicsSystem>();
    }

    GraphicsSystem() = default;

    ~GraphicsSystem() override
    {
	    GUI::NinePatch::reset();
    	AssetRenderer::reset();
        Skin::reset();
        HAL::Texture::reset_manager();
        HAL::pixel_shader::reset_manager();
        HAL::vertex_shader::reset_manager();
        HAL::domain_shader::reset_manager();
        HAL::hull_shader::reset_manager();
        HAL::geometry_shader::reset_manager();
        HAL::compute_shader::reset_manager();
        GUI::Elements::FlowGraph::manager::reset();
        Profiler::reset();
        Fonts::FontSystem::reset();
        RTX::reset();
        TextureAssetRenderer::reset();
        AssetManager::reset();
        materials::PipelineManager::reset();
        universal_nodes_manager::reset();
        universal_mesh_instance_manager::reset();
        universal_material_info_part_manager::reset();
        universal_rtx_manager::reset();
    	RenderSystem::get().device().stop_all();
        RenderSystem::reset();
    }

    static HAL::Device& device() { return RenderSystem::get().device(); }
};
