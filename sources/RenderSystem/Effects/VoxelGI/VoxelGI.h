#pragma once



class GBufferDownsampler;

class Texture3DMultiTiles
{
	update_tiling_info tilings_info;
public:

	Render::Texture::ptr tex_dynamic;
	Render::Texture::ptr tex_static;


	Render::Texture::ptr tex_result;


	void flush(CommandList& list)
	{
		tilings_info.resource = tex_result.get();
		list.update_tilings(std::move(tilings_info));
	}


	void set(CD3DX12_RESOURCE_DESC desc)
	{
		tex_dynamic.reset(new Render::Texture(desc, ResourceState::PIXEL_SHADER_RESOURCE, Render::HeapType::RESERVED));
		tex_static.reset(new Render::Texture(desc, ResourceState::PIXEL_SHADER_RESOURCE, Render::HeapType::RESERVED));

		tex_result.reset(new Render::Texture(desc, ResourceState::PIXEL_SHADER_RESOURCE, Render::HeapType::RESERVED));


		tex_dynamic->on_load = [this](ivec4 pos) {
			auto heap_pos = tex_dynamic->tiles[0][pos].heap_position;
			heap_pos.handle = ResourceHandle();
			tex_result->map_tile(tilings_info, pos, heap_pos);
		};


		tex_dynamic->on_zero = [this](ivec4 pos) {
			if (tex_static->is_mapped(pos.xyz, pos.w))
			{
				auto heap_pos = tex_static->tiles[0][pos].heap_position;
				heap_pos.handle = ResourceHandle();
				tex_result->map_tile(tilings_info, pos, heap_pos);
			}
			else
			{
				tex_result->zero_tile(tilings_info, pos, 0);
			}
		};


		tex_static->on_load = [this](ivec4 pos) {

			if (!tex_dynamic->is_mapped(pos.xyz, pos.w))
			{
				auto heap_pos = tex_static->tiles[0][pos].heap_position;
				heap_pos.handle = ResourceHandle();
				tex_result->map_tile(tilings_info, pos, heap_pos);
			}
			
		};

		tex_static->on_zero = [this](ivec4 pos) {

			if (!tex_dynamic->is_mapped(pos.xyz, pos.w))
			{
				tex_result->zero_tile(tilings_info, pos, 0);
			}

		};

	}


	void zero_tiles(CommandList& list)
	{
		tex_dynamic->zero_tiles(list);
		tex_static->zero_tiles(list);

		tilings_info.tiles.clear();
	//	flush(list);
	}
};



class Texture3DRefTiles
{
	update_tiling_info tilings_info;

	grid<ivec3, bool> static_tiles;
	grid<ivec3, bool> dynamic_tiles;

public:
	Render::Texture::ptr tex_result;


	void flush(CommandList& list)
	{
		tilings_info.resource = tex_result.get();
		list.update_tilings(std::move(tilings_info));
	}


	void set(CD3DX12_RESOURCE_DESC desc)
	{
		tex_result.reset(new Render::Texture(desc, ResourceState::PIXEL_SHADER_RESOURCE, Render::HeapType::RESERVED));

		static_tiles.resize(tex_result->get_tiles_count(), 0);
		dynamic_tiles.resize(tex_result->get_tiles_count(), 0);

	}


	void load_static(std::list<ivec3> &tiles )
	{
		for (auto& pos : tiles)
		{
			static_tiles[pos] = true;

			if (!dynamic_tiles[pos])
			{
				tex_result->load_tile(tilings_info, pos, 0 , true);
			}
		}
	}

	void zero_static(std::list<ivec3>& tiles)
	{
		for (auto& pos : tiles)
		{
			static_tiles[pos] = false;
			if (!dynamic_tiles[pos])
			{
				tex_result->zero_tile(tilings_info, pos, 0);
			}
		}
	}

	void load_dynamic(std::list<ivec3>& tiles)
	{
		for (auto& pos : tiles)
		{
			dynamic_tiles[pos] = true;

			if (!static_tiles[pos])
			{
				tex_result->load_tile(tilings_info, pos, 0, true);
			}
		}
	}

	void zero_dynamic(std::list<ivec3>& tiles)
	{
		for (auto& pos : tiles)
		{
			dynamic_tiles[pos] = false;
			if (!static_tiles[pos])
			{
				tex_result->zero_tile(tilings_info, pos, 0);
			}
		}
	}



	void zero_tiles(CommandList& list)
	{
		tilings_info.tiles.clear();

		tex_result->zero_tiles(list);
		

		static_tiles.fill(false);
		dynamic_tiles.fill(false);
	}
};




class VoxelGI :public Events::prop_handler, public FrameGraphGenerator, VariableContext
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
	
	struct EyeData:public prop_handler
	{
		Render::Texture::ptr downsampled_reflection;
		Render::Texture::ptr current_gi_texture;

		Events::prop<ivec2> size;
		EyeData();
	};

	IndirectCommand dispatch_command;

	bool recreate_static = false;
public:
	using ptr = std::shared_ptr<VoxelGI>;
//	Render::StructuredBuffer<uint2>::ptr hi;
//	Render::StructuredBuffer<uint2>::ptr low;

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
	void start_new(Render::CommandList& list);

	VoxelGI(Scene::ptr& scene);
	void voxelize(MeshRenderContext::ptr& context, main_renderer* r, FrameGraph& graph);
	void generate(MeshRenderContext::ptr& context, main_renderer::ptr r, PSSM& pssm);

	void lighting(FrameGraph& graph);
	void mipmapping(FrameGraph& graph);
	void screen(FrameGraph& graph);
	void screen_reflection(FrameGraph& graph);


	virtual void generate(FrameGraph& graph) override;
	virtual void voxelize(FrameGraph& graph) ;
	virtual void debug(FrameGraph& graph);

};