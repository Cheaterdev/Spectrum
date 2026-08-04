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
		// Per-node preview capture PSO (see material_preview.sig). Fully
		// decoupled from the passes above: own root-signature slot, own
		// harness HLSL, never touched by the production render path.
		PSOS::MaterialPreview::ptr preview;
	public:
		using ptr = std::shared_ptr<PipelinePasses>;
		PipelinePasses() = default;
		PipelinePasses(UINT id, std::string pixel, std::string tess, std::string voxel, std::string raytracing, std::string preview_source, MaterialContext::ptr context);

		HAL::library_shader::ptr  raytrace_lib;

		void set(RENDER_TYPE render_type, MESH_TYPE type, HAL::GraphicsContext& graphics, bool hiz_occlusion) override;

		PSOS::MaterialPreview::ptr get_preview() { return preview; }
	private:

		SERIALIZE()
		{
			SAVE_PARENT(Pipeline);

			ar& NVP(gbuffer);
			ar& NVP(depth_draw);
			ar& NVP(voxelization);
			// preview intentionally not serialized: it's editor-only, and
			// WrapperMap's cereal path (HAL.PipelineState.ixx) is hardcoded
			// to HAL::PipelineState, not the ComputePipelineState this PSO
			// uses. generate_material() rebuilds it fresh on the next edit.

		}
	};

	class PipelineManager :public Singleton<PipelineManager>, TypedObject<PipelineManager>
	{
		std::map<unsigned int, Pipeline::ptr> pipelines;
		std::mutex m;
	public:
		Pipeline::ptr get_pipeline(Pipeline::ptr orig);
		Pipeline::ptr get_pipeline(std::string pixel, std::string tess, std::string voxel, std::string raytracing, std::string preview, MaterialContext::ptr context);

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
		BinaryAsset::ref include_file_preview;
		uint32_t preview_header_version = 0;

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

		// Editor-only per-node preview capture: one small array texture, one
		// slice per node (see MaterialContext::get_preview_slot_count()).
		// Rebuilt/redispatched every time the graph regenerates.
		static constexpr int preview_resolution = 64;
		HAL::Texture::ptr preview_results;
		std::vector<HAL::Texture2DView> preview_slice_views;
		void update_preview_textures();

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

		size_t  get_id();

		MaterialGraph::ptr get_graph();

		Slots::MaterialInfo& get_render_info();


		void on_graph_changed();
		void generate_material();

		// Per-node preview capture (editor-only; see material_preview.sig).
		// Returns null if the graph hasn't produced a preview PSO yet.
		PSOS::MaterialPreview::ptr get_preview_pso();
		int get_preview_slot(::FlowGraph::Node* node);
		int get_preview_slot_count();
		void render_preview(HAL::ComputeContext& compute, HLSL::RWTexture2DArray<float4> results, ivec2 res);

		// Empty (default-constructed) Texture2DView if the node has no preview
		// yet (never generated, or a graph/function container node).
		HAL::Texture2DView get_preview_slice_view(::FlowGraph::Node* node);

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
			ar& NVP(include_file_preview);

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