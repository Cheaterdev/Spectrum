export module Graphics:VoxelGI;
import <RenderSystem.h>;

import :PSSM;
import :VisibilityBuffer;
import :TileDynamicGenerator;
import :GPUTilesBuffer;
import :Scene;
import :MeshRenderer;
import :ReflectionDenoiser;
import :MipMapGenerator;
import :FrameGraphContext;
		import :RTX;

import FrameGraph;
import HAL;
#include "profiling/macros.h"

//class GBufferDownsampler;

using namespace HAL;
using namespace FrameGraph;


export class Texture3DMultiTiles
{
	HAL::update_tiling_info tilings_info;

public:
	HAL::Texture::ptr tex_dynamic;
	HAL::Texture::ptr tex_static;


	HAL::Texture::ptr tex_result;


	void flush(HAL::CommandList& list)
	{
		tilings_info.resource = tex_result->resource.get();
		list.update_tilings(std::move(tilings_info));
	}


	void set(HAL::ResourceDesc desc)
	{
		desc.Flags |= ResFlags::Virtual | ResFlags::DisableStateTracking;
		tex_dynamic.reset(new HAL::Texture(desc, TextureLayout::SHADER_RESOURCE));
		tex_static.reset(new HAL::Texture(desc, TextureLayout::SHADER_RESOURCE));

		tex_result.reset(new HAL::Texture(desc, TextureLayout::SHADER_RESOURCE));

		tex_dynamic->resource->set_name("tex_dynamic");

		tex_dynamic->resource->set_name("tex_static");
		tex_dynamic->resource->set_name("tex_result");

		tex_dynamic->resource->get_tiled_manager().on_load = [this](ivec4 pos)
		{
			auto heap_pos = tex_dynamic->resource->get_tiled_manager().tiles[0][pos.xyz].heap_position;
			heap_pos.handle = ResourceHandle();
			tex_result->resource->get_tiled_manager().map_tile(tilings_info, pos.xyz, heap_pos);
		};


		tex_dynamic->resource->get_tiled_manager().on_zero = [this](ivec4 pos)
		{
			if (tex_static->resource->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				auto heap_pos = tex_static->resource->get_tiled_manager().tiles[0][pos.xyz].heap_position;
				heap_pos.handle = ResourceHandle();
				tex_result->resource->get_tiled_manager().map_tile(tilings_info, pos.xyz, heap_pos);
			}
			else
			{
				tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos.xyz, 0);
			}
		};


		tex_static->resource->get_tiled_manager().on_load = [this](ivec4 pos)
		{
			if (!tex_dynamic->resource->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				auto heap_pos = tex_static->resource->get_tiled_manager().tiles[0][pos.xyz].heap_position;
				heap_pos.handle = ResourceHandle();
				tex_result->resource->get_tiled_manager().map_tile(tilings_info, pos.xyz, heap_pos);
			}
		};

		tex_static->resource->get_tiled_manager().on_zero = [this](ivec4 pos)
		{
			if (!tex_dynamic->resource->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos.xyz, 0);
			}
		};
	}


	void zero_tiles(HAL::CommandList& list)
	{
		tex_dynamic->resource->get_tiled_manager().zero_tiles((list));
		tex_static->resource->get_tiled_manager().zero_tiles((list));

		tilings_info.tiles.clear();
		//	flush(list);
	}
};


export class Texture3DRefTiles
{
	HAL::update_tiling_info tilings_info;

	grid<ivec3, bool> static_tiles;
	grid<ivec3, bool> dynamic_tiles;

public:
	HAL::Texture::ptr tex_result;


	void flush(HAL::CommandList& list)
	{
		tilings_info.resource = tex_result->resource.get();
		list.update_tilings(std::move(tilings_info));
	}


	void set(HAL::ResourceDesc desc)
	{
		desc.Flags |= ResFlags::Virtual | ResFlags::DisableStateTracking;
		tex_result.reset(new HAL::Texture(desc, TextureLayout::SHADER_RESOURCE));

		static_tiles.resize(tex_result->resource->get_tiled_manager().get_tiles_count(), 0);
		dynamic_tiles.resize(tex_result->resource->get_tiled_manager().get_tiles_count(), 0);
	}


	void load_static(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			static_tiles[pos] = true;

			if (!dynamic_tiles[pos])
			{
				tex_result->resource->get_tiled_manager().load_tile(tilings_info, pos, 0, true);
			}
		}
	}

	void zero_static(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			static_tiles[pos] = false;
			if (!dynamic_tiles[pos])
			{
				tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos, 0);
			}
		}
	}

	void load_dynamic(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			dynamic_tiles[pos] = true;

			if (!static_tiles[pos])
			{
				tex_result->resource->get_tiled_manager().load_tile(tilings_info, pos, 0, true);
			}
		}
	}

	void zero_dynamic(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			dynamic_tiles[pos] = false;
			if (!static_tiles[pos])
			{
				tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos, 0);
			}
		}
	}


	void zero_tiles(HAL::CommandList& list)
	{
		tilings_info.tiles.clear();

		tex_result->resource->get_tiled_manager().zero_tiles(list);


		static_tiles.fill(false);
		dynamic_tiles.fill(false);
	}
};


export class VoxelGI : public Events::prop_handler, VariableContext
{
public:
	enum class VISUALIZE_TYPE :int
	{
		FULL = 0,
		INDIRECT = 1,
		REFLECTION = 2,
		VOXEL = 3
	};

	float3 min;
	float3 size;

private:
//	std::shared_ptr<GBufferDownsampler> downsampler;

	VisibilityBufferUniversal::ptr visibility;
	ReflectionDenoiser reflection_denoiser;

	TileDynamicGenerator dynamic_generator_voxelizing;
	TileDynamicGenerator dynamic_generator_lighted;

	Scene::ptr& scene;

	int all_scene_regen_counter = 0;

	bool need_start_new = false;
	int gi_index = 0;
	int refl_index = 0;
	void init_states();
	int light_counter = 0;

	std::future<visibility_update> vis_update;

	struct EyeData : public prop_handler
	{
		HAL::Texture::ptr downsampled_reflection;
		HAL::Texture::ptr current_gi_texture;

		Events::prop<ivec2> size;
		EyeData();
	};

	bool recreate_static = false;

	// private render helpers (defined in VoxelGIGraph.cpp, use <RenderSystem.h> symbols)
	void voxelize(MeshRenderContext::ptr& context, main_renderer* r, Graph& graph);

	// Pass function members — bodies defined in VoxelGIGraph.cpp
	Passes::Voxelize::setup_func_type                      m_voxelize_setup;
	Passes::Voxelize::render_func_type                     m_voxelize_render;
	Passes::Lighting::setup_func_type                      m_lighting_setup;
	Passes::Lighting::render_func_type                     m_lighting_render;
	Passes::Mipmapping::setup_func_type                    m_mipmapping_setup;
	Passes::Mipmapping::render_func_type                   m_mipmapping_render;
	Passes::VoxelScreen::setup_func_type                   m_voxelscreen_setup;
	Passes::VoxelScreen::render_func_type                  m_voxelscreen_render;
	Passes::VoxelCombine::setup_func_type                  m_voxelcombine_setup;
	Passes::VoxelCombine::render_func_type                 m_voxelcombine_render;
	Passes::ScreenReflection::setup_func_type              m_screenreflection_setup;
	Passes::ScreenReflection::render_func_type             m_screenreflection_render;
	Passes::ReflectionDenoiser_Reproject::setup_func_type  m_refldenoisereproject_setup;
	Passes::ReflectionDenoiser_Reproject::render_func_type m_refldenoisereproject_render;
	Passes::ReflCombine::setup_func_type                   m_reflcombine_setup;
	Passes::ReflCombine::render_func_type                  m_reflcombine_render;
	Passes::VoxelDebug::setup_func_type                    m_voxeldebug_setup;
	Passes::VoxelDebug::render_func_type                   m_voxeldebug_render;

public:
	using ptr = std::shared_ptr<VoxelGI>;

	ivec3 lighed_to_albedo_coeff;
	std::vector<GPUTilesBuffer::ptr> gpu_tiles_buffer;

	GPUTilesBuffer::ptr albedo_tiles;

	Texture3DMultiTiles albedo;
	Texture3DMultiTiles normal;
	Texture3DRefTiles tex_lighting;

	Variable<bool> voxelize_scene = {true, "voxelize_scene", this};
	Variable<bool> light_scene = {true, "light_scene", this};
	Variable<bool> clear_scene = {true, "clear_scene", this};

	Variable<bool> use_rtx = {true, "use_rtx", this};
	Variable<bool> multiple_bounces = {true, "multiple_bounces", this};


	Variable<bool> denoiser = {true, "denoiser", this};
	Variable<bool> reflecton = {true, "reflecton", this};


	void pass_data( FrameGraph::TaskBuilder& builder)
	{
			// Always register external textures with the frame graph
		builder.pass_texture("VoxelAlbedo",        albedo.tex_result);
		builder.pass_texture("VoxelNormal",        normal.tex_result);
		builder.pass_texture("VoxelLighted",       tex_lighting.tex_result);
		builder.pass_texture("VoxelAlbedoStatic",  albedo.tex_static);
		builder.pass_texture("VoxelNormalStatic",  normal.tex_static);
		builder.pass_texture("VoxelAlbedoDynamic", albedo.tex_dynamic);
		builder.pass_texture("VoxelNormalDynamic", normal.tex_dynamic);
	}

	void resize(ivec2 size);
	void start_new(HAL::CommandList& list);

	VoxelGI(Scene::ptr& scene);
	void generate(MeshRenderContext::ptr& context, main_renderer::ptr r, PSSM& pssm);

	template<typename TPipeline>
	explicit VoxelGI(TPipeline& pipeline, Scene::ptr& scene) : VoxelGI(scene)
	{
		pipeline.voxelize.setup_func          = m_voxelize_setup;
		pipeline.voxelize.render_func         = m_voxelize_render;
		pipeline.lighting.setup_func          = m_lighting_setup;
		pipeline.lighting.render_func         = m_lighting_render;
		pipeline.mipmapping.setup_func        = m_mipmapping_setup;
		pipeline.mipmapping.render_func       = m_mipmapping_render;
		pipeline.voxelScreen.setup_func       = m_voxelscreen_setup;
		pipeline.voxelScreen.render_func      = m_voxelscreen_render;
		pipeline.voxelCombine.setup_func      = m_voxelcombine_setup;
		pipeline.voxelCombine.render_func     = m_voxelcombine_render;
		pipeline.screenReflection.setup_func  = m_screenreflection_setup;
		pipeline.screenReflection.render_func = m_screenreflection_render;
		pipeline.reflectionDenoiser_Reproject.setup_func  = m_refldenoisereproject_setup;
		pipeline.reflectionDenoiser_Reproject.render_func = m_refldenoisereproject_render;
		pipeline.reflCombine.setup_func       = m_reflcombine_setup;
		pipeline.reflCombine.render_func      = m_reflcombine_render;
		pipeline.voxelDebug.setup_func        = m_voxeldebug_setup;
		pipeline.voxelDebug.render_func       = m_voxeldebug_render;
	}
};
