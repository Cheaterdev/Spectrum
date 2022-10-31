#pragma once
#include "TiledTextures/VisibilityBuffer.h"
#include "TiledTextures/GPUTilesBuffer.h"
#include "TiledTextures/DynamicTileGenerator.h"

#include "Lighting/PSSM.h"

#include "Renderer/Renderer.h"

class GBufferDownsampler;
import Graphics;


class Texture3DMultiTiles
{
	HAL::update_tiling_info tilings_info;
public:

	HAL::Texture::ptr tex_dynamic;
	HAL::Texture::ptr tex_static;


	HAL::Texture::ptr tex_result;


	void flush(HAL::CommandList& list)
	{
		tilings_info.resource = tex_result.get();
		list.update_tilings(std::move(tilings_info));
	}


	void set(HAL::ResourceDesc desc)
	{
		tex_dynamic.reset(new HAL::Texture(desc, HAL::ResourceState::PIXEL_SHADER_RESOURCE, HAL::HeapType::RESERVED));
		tex_static.reset(new HAL::Texture(desc, HAL::ResourceState::PIXEL_SHADER_RESOURCE, HAL::HeapType::RESERVED));

		tex_result.reset(new HAL::Texture(desc, HAL::ResourceState::PIXEL_SHADER_RESOURCE, HAL::HeapType::RESERVED));


		tex_dynamic->get_tiled_manager().on_load = [this](ivec4 pos) {
			auto heap_pos = tex_dynamic->get_tiled_manager().tiles[0][pos].heap_position;
			heap_pos.handle = ResourceHandle();
			tex_result->get_tiled_manager().map_tile(tilings_info, pos, heap_pos);
		};


		tex_dynamic->get_tiled_manager().on_zero = [this](ivec4 pos) {
			if (tex_static->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				auto heap_pos = tex_static->get_tiled_manager().tiles[0][pos].heap_position;
				heap_pos.handle = ResourceHandle();
				tex_result->get_tiled_manager().map_tile(tilings_info, pos, heap_pos);
			}
			else
			{
				tex_result->get_tiled_manager().zero_tile(tilings_info, pos, 0);
			}
		};


		tex_static->get_tiled_manager().on_load = [this](ivec4 pos) {

			if (!tex_dynamic->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				auto heap_pos = tex_static->get_tiled_manager().tiles[0][pos].heap_position;
				heap_pos.handle = ResourceHandle();
				tex_result->get_tiled_manager().map_tile(tilings_info, pos, heap_pos);
			}

		};

		tex_static->get_tiled_manager().on_zero = [this](ivec4 pos) {

			if (!tex_dynamic->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				tex_result->get_tiled_manager().zero_tile(tilings_info, pos, 0);
			}

		};

	}


	void zero_tiles(HAL::CommandList& list)
	{
		tex_dynamic->get_tiled_manager().zero_tiles((list));
		tex_static->get_tiled_manager().zero_tiles((list));

		tilings_info.tiles.clear();
		//	flush(list);
	}
};



class Texture3DRefTiles
{
	HAL::update_tiling_info tilings_info;

	grid<ivec3, bool> static_tiles;
	grid<ivec3, bool> dynamic_tiles;

public:
	HAL::Texture::ptr tex_result;


	void flush(HAL::CommandList& list)
	{
		tilings_info.resource = tex_result.get();
		list.update_tilings(std::move(tilings_info));
	}


	void set(HAL::ResourceDesc desc)
	{
		tex_result.reset(new HAL::Texture(desc, HAL::ResourceState::PIXEL_SHADER_RESOURCE, HAL::HeapType::RESERVED));

		static_tiles.resize(tex_result->get_tiled_manager().get_tiles_count(), 0);
		dynamic_tiles.resize(tex_result->get_tiled_manager().get_tiles_count(), 0);

	}


	void load_static(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			static_tiles[pos] = true;

			if (!dynamic_tiles[pos])
			{
				tex_result->get_tiled_manager().load_tile(tilings_info, pos, 0, true);
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
				tex_result->get_tiled_manager().zero_tile(tilings_info, pos, 0);
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
				tex_result->get_tiled_manager().load_tile(tilings_info, pos, 0, true);
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
				tex_result->get_tiled_manager().zero_tile(tilings_info, pos, 0);
			}
		}
	}



	void zero_tiles(HAL::CommandList& list)
	{
		tilings_info.tiles.clear();

		tex_result->get_tiled_manager().zero_tiles(list);


		static_tiles.fill(false);
		dynamic_tiles.fill(false);
	}
};

using namespace FrameGraph;


class VoxelGI :public Events::prop_handler, public FrameGraph::GraphGenerator, VariableContext
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
	std::shared_ptr<GBufferDownsampler> downsampler;

	VisibilityBufferUniversal::ptr visibility;


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

	struct EyeData :public prop_handler
	{
		HAL::Texture::ptr downsampled_reflection;
		HAL::Texture::ptr current_gi_texture;

		Events::prop<ivec2> size;
		EyeData();
	};

	IndirectCommand dispatch_command;

	bool recreate_static = false;
public:
	using ptr = std::shared_ptr<VoxelGI>;
	//	HAL::StructureBuffer<uint2>::ptr hi;
	//	HAL::StructureBuffer<uint2>::ptr low;

	ivec3 lighed_to_albedo_coeff;
	std::vector<GPUTilesBuffer::ptr> gpu_tiles_buffer;

	GPUTilesBuffer::ptr albedo_tiles;

	Texture3DMultiTiles albedo;
	Texture3DMultiTiles normal;
	Texture3DRefTiles tex_lighting;

	Variable<bool> voxelize_scene = { true, "voxelize_scene", this };
	Variable<bool> light_scene = { true, "light_scene", this };
	Variable<bool> clear_scene = { true, "clear_scene",this };

	Variable<bool> use_rtx = { true, "use_rtx", this };
	Variable<bool> multiple_bounces = { true, "multiple_bounces", this };


	Variable<bool> denoiser = { true, "denoiser", this };
	Variable<bool> reflecton = { true, "reflecton", this };


	void resize(ivec2 size);
	void start_new(HAL::CommandList& list);

	VoxelGI(Scene::ptr& scene);
	void voxelize(MeshRenderContext::ptr& context, main_renderer* r, Graph& graph);
	void generate(MeshRenderContext::ptr& context, main_renderer::ptr r, PSSM& pssm);

	void lighting(Graph& graph);
	void mipmapping(Graph& graph);
	void screen(Graph& graph);
	void screen_reflection(Graph& graph);


	virtual void generate(Graph& graph) override;
	virtual void voxelize(Graph& graph);
	virtual void debug(Graph& graph);

};