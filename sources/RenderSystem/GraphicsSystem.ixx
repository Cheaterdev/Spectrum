export module Graphics:System;

import :RTX;
import :AssetRenderer;
import :TextureAsset;
import :Asset;
import :Materials.UniversalMaterial;
import :MeshAsset;
import HAL;
import GUI;
import Core;
import TextSystem;

// Owns the creation and destruction of all graphics singletons in the correct order.
// Call GraphicsSystem::create() after logging/filesystem setup; call reset() before exit.
export class GraphicsSystem : public Singleton<GraphicsSystem>
{
    friend class Singleton<GraphicsSystem>;

    GraphicsSystem()
    {
        HAL::Device::create();
        if (HAL::Device::get().is_rtx_supported())
            RTX::create();
#ifndef HAL_BACKEND_VULKAN
        AssetRenderer::create();
#endif
        AssetManager::create();
    }

    ~GraphicsSystem() override
    {
        HAL::Device::get().stop_all();
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
        AssetRenderer::reset();
        TextureAssetRenderer::reset();
        AssetManager::reset();
        materials::PipelineManager::reset();
        universal_nodes_manager::reset();
        universal_mesh_instance_manager::reset();
        universal_material_info_part_manager::reset();
        universal_rtx_manager::reset();
        HAL::Device::reset();
    }
};
