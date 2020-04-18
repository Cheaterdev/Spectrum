#pragma once



class GBufferDownsampler;

class VoxelGI :public Events::prop_handler
{
public:

	enum class VISUALIZE_TYPE :int
	{
		FULL = 0,
		INDIRECT = 1,
		REFLECTION = 2,
		VOXEL = 3
	};

private:
	std::shared_ptr<GBufferDownsampler> downsampler;

	std::array<Render::PipelineState::ptr[4], 4> screen_states;


	struct reflection
	{
		Render::PipelineState::ptr reflection_state;
		Render::PipelineState::ptr reflection_resize_state;

	};

	std::array<reflection, 4> reflection_states;

	Render::PipelineState::ptr downsampled_reflection_state;

//	Render::PipelineState::ptr draw_on_screen_state[2];

	Render::ComputePipelineState::ptr downsample_state[3];
	Render::ComputePipelineState::ptr lighting_state;
	Render::ComputePipelineState::ptr lighting_state_second_bounce;

	//Render::PipelineState::ptr resize_state;


	Render::HandleTable uav_table;
	Render::HandleTable srv_table;

	Render::HandleTable second_bunce_table;

	VisibilityBufferUniversal::ptr visibility;

	Texture3DTiledDynamic::ptr tiled_volume_normal;
	Texture3DTiledDynamic::ptr tiled_volume_lighted;


	Texture3DTiledDynamic::ptr tiled_volume_normal_static;
	Texture3DTiledDynamic::ptr tiled_volume_albedo_static;




	TileDynamicGenerator dynamic_generator_voxelizing;

	TileDynamicGenerator dynamic_generator;
	Scene::ptr& scene;

	int all_scene_regen_counter = 0;

	bool need_start_new = false;

	void init_states();
	int light_counter = 0;

	G_Buffer& buffer;

//	PlacedAllocator allocator;
	
	HandleTable gi_rtv;
	ReflectionEffectPixel reflection_effect;

	struct EyeData:public prop_handler
	{
		PlacedAllocator allocator;

		TextureSwapper gi_textures;
		TextureSwapper downsampled_light;
		Render::Texture::ptr downsampled_reflection;
		Render::Texture::ptr current_gi_texture;

		Events::prop<ivec2> size;
		EyeData();
	};
public:
	using ptr = std::shared_ptr<VoxelGI>;

	Texture3DTiledDynamic::ptr tiled_volume_albedo;
	ivec3 lighed_to_albedo_coeff;
	std::vector<GPUTilesBuffer::ptr> gpu_tiles_buffer;

	Render::Texture::ptr volume_albedo;
	Render::Texture::ptr volume_normal;
	Render::Texture::ptr volume_lighted;


	Render::Texture::ptr  ssgi_tex;

	Variable<bool> voxelize_scene = Variable<bool>(true, "voxelize_scene");
	Variable<bool> light_scene = Variable<bool>(true, "light_scene");
	Variable<bool> clear_scene = Variable<bool>(true, "clear_scene");

	Variable<VISUALIZE_TYPE> render_type = Variable<VISUALIZE_TYPE>(VISUALIZE_TYPE::FULL, "render_type");

	void resize(ivec2 size);
	void start_new();

	VoxelGI(Scene::ptr& scene, G_Buffer& buffer);
	void voxelize(MeshRenderContext::ptr& context, main_renderer::ptr r);
	void generate(MeshRenderContext::ptr& context, main_renderer::ptr r, PSSM& pssm, Enviroment enviroment);

	void lighting(MeshRenderContext::ptr& context, PSSM& pssm, Render::CubemapArrayView::ptr skymap);
	void mipmapping(MeshRenderContext::ptr& context);
	void screen(MeshRenderContext::ptr& context, scene_object::ptr scene, Render::CubemapArrayView::ptr skymap);
	void screen_reflection(MeshRenderContext::ptr& context, scene_object::ptr scene, Render::CubemapArrayView::ptr skymap);

};