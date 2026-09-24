export module Graphics:VoxelGI;
import :PSSM;
import :VSM;
import :VisibilityBuffer;
import :TileDynamicGenerator;
import :GPUTilesBuffer;
import :Scene;
import :MeshRenderer;
import :MipMapGenerator;
import :FrameGraphContext;
		import :RTX;

import FrameGraph;
import HAL;
import :Texture;

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


	void flush(HAL::CommandList& list);
	void set(HAL::ResourceDesc desc);
	void zero_tiles(HAL::CommandList& list);
};


export class Texture3DRefTiles
{
	HAL::update_tiling_info tilings_info;

	grid<ivec3, bool> static_tiles;
	grid<ivec3, bool> dynamic_tiles;

public:
	HAL::Texture::ptr tex_result;


	void flush(HAL::CommandList& list);
	void set(HAL::ResourceDesc desc);
	void load_static(std::list<uint3>& tiles);
	void zero_static(std::list<uint3>& tiles);
	void load_dynamic(std::list<uint3>& tiles);
	void zero_dynamic(std::list<uint3>& tiles);
	void zero_tiles(HAL::CommandList& list);
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

	TileDynamicGenerator dynamic_generator_voxelizing;
	TileDynamicGenerator dynamic_generator_lighted;

	Scene::ptr& scene;
	VSM& vsm;

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

	// Pass function members — bodies defined in VoxelGIGraph.cpp. Render only:
	// every one of these passes states its enable condition in voxel.prism and
	// gets a generated setup (PassSetupDefault<T>, pass_defaults.h).
	Passes::GI::Voxel::Voxelize::render_func_type                     m_voxelize_render;
	Passes::GI::Voxel::Lighting::render_func_type                     m_lighting_render;
	Passes::GI::Voxel::Mipmapping::render_func_type                   m_mipmapping_render;
	Passes::Post::Upscale::NormalRoughnessRepack::render_func_type        m_normalroughnessrepack_render;
	Passes::Reflections::ReflCombine::render_func_type                  m_reflcombine_render;
	Passes::GI::Voxel::Dev::VoxelDebug::render_func_type                   m_voxeldebug_render;
	Passes::GI::Voxel::VoxelScreen::render_func_type                  m_voxelscreen_render;
	Passes::Reflections::ScreenReflection::render_func_type             m_screenreflection_render;

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

	Variable<bool> multiple_bounces = {true, "multiple_bounces", this};

	// Now just "are reflections composited at all" -- gates the surviving
	// ReflCombine (NRD REBLUR_SPECULAR path, see [[project-nrd-integration]]);
	// the old FFX-denoised legacy path it used to also gate is gone.
	Variable<bool> reflecton = {true, "reflecton", this};


	void pass_data(FrameGraph::TaskBuilder& builder);

	// Once per frame, before graph.setup(): mirrors the Variable<bool> toggles
	// above into Table::GI::Voxel::VoxelGISelectors (voxel.prism) for the generated setups
	// to read, and does the voxel-bounds/VoxelInfo update that used to live in
	// Voxelize's own setup lambda. Both need a VoxelGI instance, which a
	// generated static setup has no way to reach.
	void update_frame(FrameGraph::Graph& graph);

	void resize(ivec2 size);
	void start_new(HAL::CommandList& list);

	VoxelGI(Scene::ptr& scene, VSM& vsm);
	void generate(MeshRenderContext::ptr& context, main_renderer::ptr r, PSSM& pssm);

	template<typename TPipeline>
	explicit VoxelGI(TPipeline& pipeline, Scene::ptr& scene, VSM& vsm) : VoxelGI(scene, vsm)
	{
		pipeline.voxelize.render_func              = m_voxelize_render;
		pipeline.lighting.render_func              = m_lighting_render;
		pipeline.mipmapping.render_func            = m_mipmapping_render;
		pipeline.normalRoughnessRepack.render_func = m_normalroughnessrepack_render;
		pipeline.reflCombine.render_func           = m_reflcombine_render;
		pipeline.voxelDebug.render_func            = m_voxeldebug_render;
		pipeline.voxelScreen.render_func           = m_voxelscreen_render;
		pipeline.screenReflection.render_func      = m_screenreflection_render;
	}
};
