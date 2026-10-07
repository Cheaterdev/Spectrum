export module Graphics:SceneFrameRenderer;

import Core;
import HAL;
import FrameGraph;

import :Camera;
import :Scene;
import :FrameGraphContext;
import :MeshAsset;
import :MeshRenderer;
import :VSM;
import :Sky;
import :SMAA;
import :BlueNoise;
import :VoxelGI;

// What a scene frame depends on from outside: the editor viewport fills it
// from its widget, camera and wall clock; tests fix it for reproducible frames.
export struct SceneFrameParams
{
	camera* cam = nullptr;
	float   dt = 0;
	ivec2   output_size = { 64, 64 };
	bool    upscale = true;   // render below output_size and upscale (g_upscaler_type)
	bool    jitter = true;    // sub-pixel camera jitter for the temporal upscalers
};

// Builds the main scene frame (Pipelines::Frame::MainPipeline) for a Scene: owns
// the pipeline and the systems wired into it (VSM, sky, blue noise, SMAA, voxel
// GI, DDGI) and the scene renderers.
//
// A frame is two calls, with the caller's own per-frame work in between:
//   begin_frame  -- contexts, viewport, camera jitter, each system's update_frame
//   (caller)     -- e.g. stencil_renderer / debug_view update_frame: they must see
//                   this frame's selectors and run before the passes are added
//   end_frame    -- add_passes, the [PreSetup] hooks, pass_data, slot generators
export class SceneFrameRenderer : public VariableContext
{
	// Declaration order is construction order: the systems below register their
	// passes into the pipeline, and VoxelGI needs VSM.
	Pipelines::Frame::MainPipeline m_pipeline;
	VSM        m_vsm;
	SkyRender  m_sky;
	BlueNoise  m_blue_noise;
	SMAA       m_smaa;

	// Grouping contexts for the mesh_renderer instances below -- mesh_renderer
	// always names itself "mesh_renderer" (see MeshRenderer.cpp), so without
	// these every instance across the app would be an identically-named
	// sibling in the Properties tree.
	VariableContext m_forward_context{ L"Main View / Forward" };
	VariableContext m_gpu_context{ L"Main View / GPU" };

	Scene::ptr          m_scene;
	main_renderer::ptr  m_forward_renderer;
	main_renderer::ptr  m_gpu_renderer;
	mesh_renderer::ptr  m_forward_meshes;
	// The main view's GBuffer mesh_renderer, whose draws walk the LOD hierarchy.
	mesh_renderer::ptr  m_main_view_meshes;
	VoxelGI::ptr        m_voxel_gi;

	SceneFrameParams m_params;
	uint64 m_frame = 0;   // TimeInfo::frame of the next begin_frame
	// FrameInfo::lodThreshold for this frame.
	float m_lod_threshold = 0;

	ivec2 m_last_logged_frame_size   = { -1, -1 };
	ivec2 m_last_logged_upscale_size = { -1, -1 };

public:
	Variable<bool>  cluster_lod      = { true, "Cluster LOD", this };
	Variable<float> lod_error_pixels = { 1.0f, "LOD error (px)", this, 0.25f, 16.0f };

	explicit SceneFrameRenderer(Scene::ptr scene);
	~SceneFrameRenderer();

	Pipelines::Frame::MainPipeline& pipeline() { return m_pipeline; }
	Scene::ptr scene() const { return m_scene; }
	VSM& vsm() { return m_vsm; }
	mesh_renderer::ptr main_view_mesh_renderer() const { return m_main_view_meshes; }
	float lod_threshold() const { return m_lod_threshold; }

	// Sun direction (towards the sun); VSM owns it, the sky reads it from there.
	void set_sun_direction(float3 dir) { m_vsm.set_position(dir); }

	void begin_frame(FrameGraph::Graph& graph, const SceneFrameParams& params);
	void end_frame(FrameGraph::Graph& graph);
};
