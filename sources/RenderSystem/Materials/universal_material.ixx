export module Graphics:Materials.UniversalMaterial;


import :Materials.Values;

import :Asset;
import :MaterialAsset;
import :BinaryAsset;
import HAL;
import Core;

using namespace HAL;
export namespace materials
{

	struct render_pass
	{
		HAL::pixel_shader::ptr ps_shader;
		HAL::domain_shader::ptr ds_shader;
		HAL::hull_shader::ptr hs_shader;

		void init_pipeline_id();


		D3D_PRIMITIVE_TOPOLOGY get_topology();


		size_t get_pipeline_id();

	private:
		size_t pipeline_id = -1;


		SERIALIZE()
		{
			ar& NVP(ps_shader);
			ar& NVP(ds_shader);
			ar& NVP(hs_shader);
		//	ar& NVP(pipeline_id);

			IF_LOAD()
				init_pipeline_id();
		}
	};
	enum PASS_TYPE
	{
		DEFERRED,
		DEPTH,
		VOXEL_DYNAMIC,
		VOXEL_STATIC,
		COUNTER
	};


	class PipelineSimple : public Pipeline
	{
		pixel_shader::ptr pixel;
	public:
		using ptr = std::shared_ptr<PipelineSimple>;
		PipelineSimple(UINT id, pixel_shader::ptr pixel = nullptr);
		PipelineSimple() = default;

		void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion) override;

	private:

		SERIALIZE()
		{
			SAVE_PARENT(Pipeline);
			ar& NVP(pixel);
		}

	};
	class PipelinePasses : public Pipeline
	{
		PSOS::GBufferDraw::ptr gbuffer;
		PSOS::Voxelization::ptr voxelization;
		PSOS::DepthDraw::ptr depth_draw;
		// Only compiled when transparent is true (see the constructor) --
		// opaque materials (the vast majority) never pay for this PSO at all
		// and keep using VSM's single shared VSMDepthDraw (no pixel shader).
		PSOS::VSMDepthDrawMaterial::ptr vsm_depth_draw;
		bool transparent = false;
	public:
		using ptr = std::shared_ptr<PipelinePasses>;
		PipelinePasses() = default;
		PipelinePasses(UINT id, std::string pixel, std::string tess, std::string voxel, std::string raytracing, MaterialContext::ptr context);

		HAL::library_shader::ptr  raytrace_lib;

		void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion) override;
		bool is_transparent() const override { return transparent; }
		// nullptr for opaque pipelines -- callers (VSM.cpp) must check
		// is_transparent() first, or just null-check this directly.
		PSOS::VSMDepthDrawMaterial::ptr get_vsm_depth_draw() const override { return vsm_depth_draw; }
	private:

		SERIALIZE()
		{
			SAVE_PARENT(Pipeline);

			ar& NVP(gbuffer);
			ar& NVP(depth_draw);
			ar& NVP(voxelization);
			ar& NVP(vsm_depth_draw);
			ar& NVP(transparent);

		}
	};

	class PipelineManager :public Singleton<PipelineManager>, TypedObject<PipelineManager>
	{
		std::map<unsigned int, Pipeline::ptr> pipelines;
		std::mutex m;
	public:
		Pipeline::ptr get_pipeline(Pipeline::ptr orig);
		Pipeline::ptr get_pipeline(std::string pixel, std::string tess, std::string voxel, std::string raytracing, MaterialContext::ptr context);

	};
	
	class command_data
	{
		UINT index;

	};

	class universal_material : public MaterialAsset, ::FlowGraph::graph_listener, public MainRTX::material
	{
			/*----------------------------------------------------------*/
			virtual	void on_register(::FlowGraph::window*)override;
		virtual	void on_remove(::FlowGraph::window*)override;

		virtual	void on_add_input(::FlowGraph::parameter*)override;
		virtual	void on_remove_input(::FlowGraph::parameter*)override;

		virtual	void on_add_output(::FlowGraph::parameter*)override;
		virtual	void on_remove_output(::FlowGraph::parameter*)override;

		virtual	void on_link(::FlowGraph::parameter*, ::FlowGraph::parameter*)override;
		virtual	void on_unlink(::FlowGraph::parameter*, ::FlowGraph::parameter*) override;
		/*----------------------------------------------------------*/

		std::vector<TextureAsset::ref> textures;

		std::vector<Uniform::ptr> tess_uniforms;
		std::list<Events::prop_handler> handlers;

		DynamicData pixel_data;

		BinaryData<MaterialGraph> graph;

		MaterialContext::ptr context;

		BinaryAsset::ref include_file;
		BinaryAsset::ref include_file_raytacing;

		bool need_update_uniforms = false;
		bool need_update_compiled = false;

		std::mutex m;
		virtual void on_asset_change(std::shared_ptr<Asset> asset) override;
		virtual void compile() override;
		universal_material();
		bool need_regenerate_material = false;

		// Header (shader include) reload versions this material was last generated
		// against; if the shared BinaryAsset advances past these we regenerate.
		uint32_t ps_header_version = 0;
		uint32_t rt_header_version = 0;

		// True when the material graph drives the opacity output (see MaterialContext).
		// Consumed later by RTXColorPass to handle refraction/blending.
		bool transparent = false;

		void generate_texture_handles();

		Slots::MaterialInfo material_info;

		std::vector<HLSL::Texture2D<float4>> texture_srvs;
		std::vector<HLSL::FeedbackTexture2DMip> texture_feedbacks;
		Pipeline::ptr pipeline;

	public:
		using ptr = s_ptr<universal_material>;
		std::vector<Uniform::ptr> ps_uniforms;
		//	Events::Event<void> on_change;
	//		const std::wstring wshader_name;

	//		HAL::library_shader::ptr raytracing_lib;
		universal_material(MaterialGraph::ptr graph);
		Slots::MaterialInfo::Compiled compiled_material_info;

		::TypedHandle<Table::MaterialCommandData::Compiled> info_handle;

		void update_rtx();
		void test();

		void update();

		UINT get_material_id();

		bool is_transparent() const { return transparent; }

		Pipeline::ptr get_pipeline();

		// Bumped whenever a material's pipeline is (re)generated or a mesh's
		// material assignment changes. Scene caches its derived material and
		// pipeline sets and compares against this to know when to rebuild --
		// without it, a shader recompile would leave a stale Pipeline::ptr in
		// Scene::pipelines.
		static std::atomic<uint64_t> pipeline_epoch;

		size_t  get_id();

		MaterialGraph::ptr get_graph();

		Slots::MaterialInfo& get_render_info();


		void on_graph_changed();
		void generate_material();

		// Per-node preview capture (editor-only; see material_preview.sig).
		// Just the raw ingredients -- building/compiling/dispatching the
		// actual preview PSO and results texture is materials::
		// MaterialPreviewSession's job (owned by the graph editor's canvas
		// while it's open), not this material's.
		//
		// preview_source_generation bumps only on a structural regenerate
		// (compiled shader text changed -- a session needs to recompile its
		// PSO). preview_generation bumps on that AND on pixel_data-only
		// updates (e.g. a slider drag -- a session just needs to redispatch
		// with its existing PSO).
		uint32_t preview_source_generation = 0;
		uint32_t preview_generation = 0;
		int get_preview_slot(::FlowGraph::Node* node);
		int get_preview_slot_count();
		ShaderSource get_preview_shader_source();
		ShaderSource get_pixel_shader_source();

		// Editor-only settings, read by main.cpp's material settings panel /
		// asset_preview_content. Purely presentational -- generate_material()/
		// compile() are entirely unaffected by these.
		bool show_scene_preview = true;

		// Shared live-3D-preview camera, in the same orbit/zoom terms
		// SceneTextureRenderer uses. Any full preview of this material
		// (asset_preview_content -- the material graph's output-node-embedded
		// preview, or an Asset Explorer preview window) writes here while the
		// user orbits/zooms it; MaterialPreviewSession's per-node 3D preview
		// thumbnails read it instead of using a fixed angle, so every node's
		// preview matches whatever view the user last set on the full one.
		vec2  preview_orbit = { 0.785398f, 0.35f }; // yaw, pitch -- ~pi/4 yaw, slight pitch
		float preview_zoom  = 1.5f;

		// Binds this material's current texture/uniform data and dispatches
		// into results using the caller-supplied preview PSO (owned by
		// whoever is hosting the preview, e.g. MaterialPreviewSession -- this
		// material only knows how to feed itself into one).
		void render_preview(HAL::ComputeContext& compute, PSOS::MaterialPreview::ptr preview_pso, HLSL::RWTexture2DArray<float4> results, ivec2 res);

		// Same idea, 3D mode: binds this material's data for a direct
		// dispatch_mesh draw (see MaterialPreviewSession::dispatch) instead
		// of a compute dispatch -- the PSO/mesh submission is the caller's
		// job, this only binds MaterialPreviewInfo onto the graphics context.
		void render_preview_3d(HAL::GraphicsContext& graphics, PSOS::MaterialPreview3D::ptr preview_pso, HLSL::RWTexture2DArray<float4> results);

		virtual void set(MESH_TYPE type, MeshRenderContext::ptr&) override;
		virtual void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion) override;
	private:
		SERIALIZE() {
			SAVE_PARENT(MaterialAsset);
			ar& NVP(textures);
			////////////////////////////////////////////////////////////////////////////	ar& NVP(passes);
			ar& NVP(graph);
			ar& NVP(include_file);
			ar& NVP(include_file_raytacing);

			ar& NVP(ps_uniforms);
			ar& NVP(tess_uniforms);
			ar& NVP(pipeline);


			ar& NVP(raytracing_lib);


			IF_LOAD()
			{
				auto new_pip = PipelineManager::get().get_pipeline(pipeline);

				pipeline = nullptr;
				pipeline = new_pip;
			}


			IF_LOAD()
			{
				compile();
			}
		}

	};

}
//CEREAL_FORCE_DYNAMIC_INIT(myclasses)

// REGISTER_TYPE(materials::Pipeline);
// REGISTER_TYPE(materials::PipelinePasses);
// REGISTER_TYPE(materials::PipelineSimple);