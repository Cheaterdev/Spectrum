#pragma once
#include "Materials/Values.h"
#include "Context/Context.h"
#include "Assets/BinaryAsset.h"
#include "Assets/MaterialAsset.h"
import Buffer;

import Descriptors;
import Concepts;
import Layout;
import SIG;
import crc32;

namespace materials
{
	
	struct render_pass
	{
		Render::pixel_shader::ptr ps_shader;
		Render::domain_shader::ptr ds_shader;
		Render::hull_shader::ptr hs_shader;

		void init_pipeline_id()
		{
			pipeline_id = 2166136261U;
			pipeline_id = HashIterate(ps_shader ? ps_shader->id : 0, pipeline_id);
			pipeline_id = HashIterate(ds_shader ? ds_shader->id : 0, pipeline_id);
			pipeline_id = HashIterate(hs_shader ? hs_shader->id : 0, pipeline_id);
		}


		D3D_PRIMITIVE_TOPOLOGY get_topology();


		size_t get_pipeline_id();

	private:
		size_t pipeline_id = -1;
	

		SERIALIZE()
		{
			ar& NVP(ps_shader);
			ar& NVP(ds_shader);
			ar& NVP(hs_shader);

			if (Archive::is_loading::value)
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


	class PipelineSimple :public Pipeline
	{
		pixel_shader::ptr pixel;
	public:
		using ptr = std::shared_ptr<PipelineSimple>;
		PipelineSimple(UINT id, pixel_shader::ptr pixel = nullptr) :Pipeline(id)
		{

		}PipelineSimple() = default;

		void set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc& pipeline) override {

			pipeline.pixel = pixel;
		}

	private:

		SERIALIZE()
		{
			ar& NVP(boost::serialization::base_object<Pipeline>(*this));
			ar& NVP(pixel);
		}

	};
	class PipelinePasses:public Pipeline
	{

		std::vector<render_pass> passes;
		
		render_pass& get_pass(RENDER_TYPE render_type, MESH_TYPE type)
		{

			render_pass* pass = nullptr;

			if (render_type == RENDER_TYPE::PIXEL)
				pass = &passes[PASS_TYPE::DEFERRED];
			else
				if (type == MESH_TYPE::STATIC)
					pass = &passes[PASS_TYPE::VOXEL_STATIC];
				else
					pass = &passes[PASS_TYPE::VOXEL_DYNAMIC];
			return *pass;
		}


	public:
		using ptr = std::shared_ptr<PipelinePasses>;
		PipelinePasses() = default;
		PipelinePasses(UINT id, std::vector<render_pass> & passes):Pipeline(id),passes(passes)
		{

		}
		Render::library_shader::ptr  raytrace_lib;

		void set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc& pipeline) override
		{
			auto pass = &get_pass(render_type, type);

			pipeline.pixel = pass->ps_shader;
			pipeline.domain = pass->ds_shader;
			pipeline.hull = pass->hs_shader;

			if (pass->ds_shader)
			{
				pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
			}
			else
			{
				pipeline.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			}

		}
	private:

		SERIALIZE()
		{
			ar& NVP(boost::serialization::base_object<Pipeline>(*this));
			ar& NVP(passes);
		}
	};

	class PipelineManager :public Singleton<PipelineManager>
	{
		std::map<unsigned int, Pipeline::ptr> pipelines;
		std::mutex m;
	public:
		Pipeline::ptr get_pipeline( Pipeline::ptr orig)
		{
			std::lock_guard<std::mutex> g(m);

			auto& pip = pipelines[orig->hash];
			if (!pip)
				pip = orig;

			return pip;
		}
		Pipeline::ptr get_pipeline(std::string pixel, std::string tess, std::string voxel,std::string raytracing, MaterialContext::ptr context)
		{
			std::lock_guard<std::mutex> g(m);
			auto hash = crc32(pixel+tess);
			auto&& pip = pipelines[hash];


			if (!pip)
			{
				std::vector<render_pass> passes;
				passes.resize(PASS_TYPE::COUNTER);

				passes[PASS_TYPE::DEFERRED].ps_shader = Render::pixel_shader::get_resource({ pixel, "PS", 0,context->get_pixel_result().macros, true });// create_from_memory(pixel, "PS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, context->get_pixel_result().macros);
				
				if (!tess.empty()) {
					passes[PASS_TYPE::DEFERRED].hs_shader = Render::hull_shader::get_resource({ tess, "HS", 0,context->get_tess_result().macros, true });//  Render::hull_shader::create_from_memory(tess, "HS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, context->get_tess_result().macros);
					passes[PASS_TYPE::DEFERRED].ds_shader = Render::domain_shader::get_resource({ tess, "DS", 0,context->get_tess_result().macros, true });// Render::domain_shader::create_from_memory(tess, "DS", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, context->get_tess_result().macros);

				}
				
				if (!voxel.empty()) {
					auto macros = context->get_voxel_result().macros;
					passes[PASS_TYPE::VOXEL_STATIC].ps_shader = Render::pixel_shader::get_resource({ pixel, "PS_VOXEL", 0,macros, true });//  Render::pixel_shader::create_from_memory(voxel, "PS_VOXEL", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, macros);

					macros.emplace_back("VOXEL_DYNAMIC", "1");
					passes[PASS_TYPE::VOXEL_DYNAMIC].ps_shader = Render::pixel_shader::get_resource({ pixel, "PS_VOXEL", 0,macros, true });//  Render::pixel_shader::create_from_memory(voxel, "PS_VOXEL", D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES, macros);


					passes[PASS_TYPE::VOXEL_DYNAMIC].hs_shader = passes[PASS_TYPE::VOXEL_STATIC].hs_shader = passes[PASS_TYPE::DEFERRED].hs_shader;
					passes[PASS_TYPE::VOXEL_DYNAMIC].ds_shader = passes[PASS_TYPE::VOXEL_STATIC].ds_shader = passes[PASS_TYPE::DEFERRED].ds_shader;
				}

				
				auto pipeline = std::make_shared<PipelinePasses>((UINT)pipelines.size(), passes);
				pipeline->raytrace_lib = Render::library_shader::get_resource({ raytracing, "" , 0, context->hit_shader.macros, true });

				pipeline->hash = hash;


				pip = pipeline;
			}

			return pip;
		}

	};

	class universal_material_manager :public Singleton<universal_material_manager>, protected DataAllocator<Render::Handle, ::CommonAllocator>
	{
		std::vector<Render::Handle> textures_data;
	public:
		TypedHandle<Render::Handle> allocate(size_t size)
		{
			TypedHandle<Render::Handle> result = Allocate(size);
			textures_data.resize(get_max_usage());
			return result;
		}
		std::vector<Render::Handle>& get_textures()
		{
			return textures_data;
		}
		virtual	std::span<Render::Handle> aquire(size_t offset, size_t size) override {

			return { textures_data.data()+offset, size };
		}


		virtual	void write(size_t offset, std::vector<Render::Handle>& v)  override {
			memcpy(textures_data.data() + offset, v.data(), sizeof(Render::Handle) * v.size());
		}

		virtual void on_free(size_t from, size_t to) override {

			for(auto i= from;i<to;i++)
			{
				textures_data[i] = Render::Handle();			
			}
		}

		

		universal_material_manager(): DataAllocator<Render::Handle, ::CommonAllocator>(4096)
		{
		}
	};

	class command_data
	{
		UINT index;
		
	};

    class universal_material : public MaterialAsset, ::FlowGraph::graph_listener, public MainRTX::material
    {
            LEAK_TEST(universal_material)
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


            std::vector<TextureSRVParams::ptr> textures;	
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

			void generate_texture_handles();

			Slots::MaterialInfo material_info;
		
			TypedHandle<Render::Handle> textures_handle;
			Render::HandleTable textures_handles;
			Pipeline::ptr pipeline;
    
        public:
            using ptr = s_ptr<universal_material>;
			std::vector<Uniform::ptr> ps_uniforms;
		//	Events::Event<void> on_change;
	//		const std::wstring wshader_name;
		
	//		Render::library_shader::ptr raytracing_lib;
            universal_material(MaterialGraph::ptr graph);
			Slots::MaterialInfo::Compiled compiled_material_info;

			TypedHandle<Table::MaterialCommandData::CB> info_handle;
    	
			void update_rtx();
			void test();

            void update();
        
			UINT get_material_id()
			{
				return (UINT)info_handle.get_offset();
			}

			Pipeline::ptr get_pipeline() {
				return pipeline;
			}

			size_t  get_id();

            MaterialGraph::ptr get_graph();

			Slots::MaterialInfo& get_render_info() {
				return material_info;
			}

	
            void on_graph_changed();
            void generate_material();

            virtual void set(MESH_TYPE type, MeshRenderContext::ptr&) override{}
			virtual void set(RENDER_TYPE render_type, MESH_TYPE type, Render::PipelineStateDesc &pipeline) override{}
        private:
			SERIALIZE();

    };
	
}

CEREAL_REGISTER_TYPE(materials::universal_material);

CEREAL_REGISTER_TYPE(materials::Pipeline);
CEREAL_REGISTER_TYPE(materials::PipelinePasses);
CEREAL_REGISTER_TYPE(materials::PipelineSimple);