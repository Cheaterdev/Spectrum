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
import TextEngine;

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
        { std::ofstream("teardown.temp", std::ios::app) << "nvidia::NRD::reset();\n" << std::flush; } // TEMP
        nvidia::NRD::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "nvidia::Streamline::reset();\n" << std::flush; } // TEMP
        nvidia::Streamline::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "GUI::NinePatch::reset();\n" << std::flush; } // TEMP
	    GUI::NinePatch::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "AssetRenderer::reset();\n" << std::flush; } // TEMP
    	AssetRenderer::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "Skin::reset();\n" << std::flush; } // TEMP
        Skin::reset();
        HAL::Texture::reset_manager();
        HAL::pixel_shader::reset_manager();
        HAL::vertex_shader::reset_manager();
        HAL::domain_shader::reset_manager();
        HAL::hull_shader::reset_manager();
        HAL::geometry_shader::reset_manager();
        HAL::compute_shader::reset_manager();
        { std::ofstream("teardown.temp", std::ios::app) << "GUI::Elements::FlowGraph::manager::reset();\n" << std::flush; } // TEMP
        GUI::Elements::FlowGraph::manager::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "Profiler::reset();\n" << std::flush; } // TEMP
        Profiler::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "Text::Engine::reset();\n" << std::flush; } // TEMP
        Text::Engine::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "RTX::reset();\n" << std::flush; } // TEMP
        RTX::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "TextureAssetRenderer::reset();\n" << std::flush; } // TEMP
        TextureAssetRenderer::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "AssetManager::reset();\n" << std::flush; } // TEMP
        AssetManager::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "materials::PipelineManager::reset();\n" << std::flush; } // TEMP
        materials::PipelineManager::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "universal_nodes_manager::reset();\n" << std::flush; } // TEMP
        universal_nodes_manager::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "universal_mesh_instance_manager::reset();\n" << std::flush; } // TEMP
        universal_mesh_instance_manager::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "universal_material_info_part_manager::reset();\n" << std::flush; } // TEMP
        universal_material_info_part_manager::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "universal_rtx_manager::reset();\n" << std::flush; } // TEMP
        universal_rtx_manager::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "universal_meshlet_mask_manager::reset();\n" << std::flush; } // TEMP
        universal_meshlet_mask_manager::reset();
        { std::ofstream("teardown.temp", std::ios::app) << "RenderSystem::get().device().stop_all();\n" << std::flush; } // TEMP
    	RenderSystem::get().device().stop_all();
        { std::ofstream("teardown.temp", std::ios::app) << "RenderSystem::reset();\n" << std::flush; } // TEMP
        RenderSystem::reset();
    }

    static HAL::Device& device() { return RenderSystem::get().device(); }
};
