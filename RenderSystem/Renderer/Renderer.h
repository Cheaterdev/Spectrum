

class renderer
{

    public:
        virtual ~renderer()
        {
        }

        using ptr = s_ptr<renderer>;

        virtual  bool add_object(Render::renderable* obj) = 0;
        virtual void render(MeshRenderContext::ptr c, scene_object::ptr obj) = 0;;

		virtual void iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f)=0;
};
struct mat_info
{
	int index;
	int draw_count;
};
#include "Signature.h"


#pragma pack(push, 1)
struct IndirectCommand
{

	decltype(GPUMeshSignature<Indirect>::mat_const)  mat_const;
	decltype(GPUMeshSignature<Indirect>::vertex_buffer)  vertex_buffer; 
	decltype(GPUMeshSignature<Indirect>::mat_texture_offsets)  mat_texture_offsets;
	decltype(Descriptors::index_buffer)::IndirectType  index_buffer;
	decltype(Descriptors::draw_indirect)::IndirectType  draw_indirect;

	static std::vector<D3D12_INDIRECT_ARGUMENT_DESC> get_desc()
	{
		return {
			decltype(GPUMeshSignature<Signature>::mat_const)::create_indirect(),
			decltype(GPUMeshSignature<Signature>::vertex_buffer)::create_indirect(),
			decltype(GPUMeshSignature<Signature>::mat_texture_offsets)::create_indirect(),
			decltype(Descriptors::index_buffer)::create_indirect(),
			decltype(Descriptors::draw_indirect)::create_indirect()
		};
	}

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
        /*    ar& NVP(material_cb_pixel);
            ar& NVP(vb_srv);
            ar& NVP(material.texture_offset);
			ar& NVP(material.node_offset);
            ar& NVP(index_buffer.SizeInBytes);
            ar& NVP(index_buffer.BufferLocation);
            ar& NVP(index_buffer.Format);
            ar& NVP(drawArguments.IndexCountPerInstance);
            ar& NVP(drawArguments.BaseVertexLocation);
            ar& NVP(drawArguments.InstanceCount);
            ar& NVP(drawArguments.StartIndexLocation);
            ar& NVP(drawArguments.StartInstanceLocation);*/
        }
};
#pragma pack(pop)
class main_renderer
{
        std::set<renderer::ptr> renderers;

    public:
        using ptr = s_ptr<main_renderer>;


        void register_renderer(renderer::ptr r)
        {
            renderers.insert(r);
        }

        template<class T>
        void register_renderer(T r)
        {
            renderers.insert(std::static_pointer_cast<renderer>(r));
        }
        void render(MeshRenderContext::ptr c, scene_object::ptr obj)
        {
            /*  obj->iterate([&](scene_object * node)
              {
                  //  bool node_inside = node->is_node_inside(*c->cam);
                  //    if (!node_inside)
                  //        return false;
                  Render::renderable* render_object = dynamic_cast<Render::renderable*>(node);

                  if (render_object && node->is_inside(*c->cam))
                  {
                      for (auto && r : renderers)
                      {
                          if (r->add_object(render_object))
                              break;
                      }
                  }

                  return true;
              });
            */
            for (auto && r : renderers)
                r->render(c, obj);
        }


		void iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)>f)
		{
			for (auto && r : renderers)
				r->iterate(mesh_type, f);
		}
};
struct pipeline_draws
{
    // std::vector<MeshAssetInstance::render_info> draws;
    Render::PipelineState::ptr pipeline;
  //  UINT draw_count = 0;
   // UINT index = 0;
    D3D_PRIMITIVE_TOPOLOGY topology;
};

class DynamicSizeUAVBuffer
{
        std::array<Render::StructuredBuffer<IndirectCommand>::ptr, 16> command_buffers;
        std::array<UINT, 16> max_counts;

        std::list<Render::StructuredBuffer<IndirectCommand>::ptr> old_buffers;

		Render::HandleTable handles;
		Render::HandleTable  null_uav;

    public:
        void new_frame()
        {
            old_buffers.clear();
        }
        void process_pipeline(std::map<size_t, mat_info>& draws)
        {
       //     bool changed = false;

            for (auto& m : max_counts)
                m = 0;

            for (auto& p : draws)
            {
                UINT index = (p.second.index - 1) % command_buffers.size();
                UINT count = p.second.draw_count;
				if(p.second.draw_count >0)
                max_counts[index] = std::max(max_counts[index], count);
            }

            for (UINT i = 0; i < command_buffers.size(); i++)
            {
                auto& current = command_buffers[i];

                if (/*max_counts[i] && */(!current || current->get_count() < max_counts[i]))
                {
                    if (current)
                        old_buffers.push_back(current);

                //    changed = true;
                    current = std::make_shared<Render::StructuredBuffer<IndirectCommand>>(std::max(2048u, max_counts[i]), Render::counterType::HELP_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
					current->place_uav(handles[i]);
                }
            }

          /*  if (changed)
            {
             //   current_handles = !current_handles;

                for (UINT i = 0; i < std::min(command_buffers.size(), draws.size()); i++)
                    command_buffers[i]->place_uav(handles[i]);

				for (UINT i = std::min(command_buffers.size(), draws.size()); i < command_buffers.size(); i++)
				{
					handles[i].place(null_uav[0]);
				}
				
            }*/
        }
        Render::GPUBuffer::ptr clear_buffer;
        DynamicSizeUAVBuffer();

        void start_write(Render::CommandList& list, int binded_count);
        void start_indirect(Render::CommandList& list, int binded_count);


        Render::HandleTable get_handles();
        Render::StructuredBuffer<IndirectCommand>::ptr& get_buffer(UINT i)
        {
            return command_buffers[i];
        }

    
};

struct _voxel_info
{
	vec4 voxel_min;
	vec4 voxel_size;
	ivec4 voxel_map_size;
	ivec4 voxel_tiles_count;
	ivec4 voxels_per_tile;
};

class mesh_renderer : public renderer, public Events::prop_handler
{
      
	_voxel_info voxel_info;
        vertex_transform::ptr transformer;
        Render::vertex_shader::ptr shader;
        Render::geometry_shader::ptr voxel_geometry_shader;

        
		TextureAsset::ptr best_fit_normals;

        //    MeshRenderContext::ptr mesh_render_context;
        virtual bool add_object(Render::renderable* obj) override
        {
            auto instance = dynamic_cast<MeshAssetInstance*>(obj);
            //   if (instance)
            //    meshes.push_back(instance);
            /*  if (node)
                  nodes.push_back(node);
              else
              {
                  shared_mesh_object* shared_mesh = dynamic_cast<shared_mesh_object*>(obj);
                  instanced_meshes[shared_mesh->mesh].push_back(shared_mesh);
              }*/
            return true;
        }

        virtual void render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj) override;
		Scene::ptr scene;
		std::set<MeshAssetInstance*> static_objects;
		std::set<MeshAssetInstance*> dynamic_objects;
		void iterate(MESH_TYPE mesh_type,  std::function<void(scene_object::ptr&)> f) override;
	
    public:
        unsigned int rendered_simple;
        unsigned int rendered_instanced;
        unsigned int instances_count;

		Variable<bool> use_parrallel = Variable<bool>(false,"use_parrallel");
		Variable<bool> use_cpu_culling = Variable<bool>(false, "use_cpu_culling");
		Variable<bool> use_gpu_culling = Variable<bool>(false, "use_gpu_culling");
		Variable<bool> clear_depth = Variable<bool>(true, "clear_depth");
	
        Render::RootSignature::ptr my_signature;

        using ptr = s_ptr<mesh_renderer>;
        mesh_renderer(Scene::ptr scene = nullptr);
};



class gpu_mesh_renderer : public renderer, public Events::prop_handler
{
	//  std::map<std::shared_ptr<Mesh>, std::list<shared_mesh_object*>> instanced_meshes;
	//  std::list<mesh_node*> nodes;

	// Render::Buffer<mat4x4>::ptr instances_buffer;
protected:
	struct instance
	{
		box b;
		unsigned int instance_id;
		unsigned int pipeline_id;
		mat4x4 node;
		mat4x4 inv;
	};

	struct second_draw
	{
		UINT constant;
		D3D12_DRAW_INDEXED_ARGUMENTS args;
	};

	struct gather_second
	{
		UINT constant;
		D3D12_DISPATCH_ARGUMENTS args;
	};

	//	Render::FrameStorageConstBuffer<voxel_info> voxel_buffer;



	vertex_transform::ptr transformer;
	Render::vertex_shader::ptr shader;
	Render::geometry_shader::ptr voxel_geometry_shader;

	Render::StructuredBuffer<vec4>::ptr vertex_buffer;
	Render::IndexBuffer::ptr index_buffer;
	Render::PipelineState::ptr state;

	Render::StructuredBuffer<UINT>::ptr visible_id_buffer;
	//Render::StructuredBuffer<instance>::ptr visible_id_buffer;

	ComPtr<ID3D12CommandSignature> m_commandSignature;
	ComPtr<ID3D12CommandSignature> second_draw_commands;
	ComPtr<ID3D12CommandSignature> command_dispatch;


	//     Render::FrameStorage<IndirectCommand>::ptr cpu_commands;
	//    Render::FrameStorage<instance>::ptr cpu_instances;
	Render::StructuredBuffer<instance>::ptr invisible_commands;

	Render::GPUBuffer::ptr second_draw_arguments;

	Render::GPUBuffer::ptr second_draw_dispatch;
	TextureAsset::ptr best_fit_normals;

	//   Render::GPUBuffer::ptr gpu_commands;

	// Render::HandleTable handles;


	DynamicSizeUAVBuffer buffers;


	enum GATHER_TYPE
	{
		ALL_GOOD = 1,
		OVERRIDED_MATERIAL = 2
	};
	std::array<Render::ComputePipelineState::ptr,4> compute_state;
	Render::ComputePipelineState::ptr gather_invisible;


	Render::ComputePipelineState::ptr dispatch_init_pipeline;

	//    MeshRenderContext::ptr mesh_render_context;
	virtual bool add_object(Render::renderable* obj) override
	{
		auto instance = dynamic_cast<MeshAssetInstance*>(obj);
		//   if (instance)
		//    meshes.push_back(instance);
		/*  if (node)
		nodes.push_back(node);
		else
		{
		shared_mesh_object* shared_mesh = dynamic_cast<shared_mesh_object*>(obj);
		instanced_meshes[shared_mesh->mesh].push_back(shared_mesh);
		}*/
		return true;
	}

	virtual void render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj) override;
	Scene::ptr scene;
	std::set<MeshAssetInstance*> static_objects;
	std::set<MeshAssetInstance*> dynamic_objects;
	void iterate(MESH_TYPE mesh_type, std::function<void(scene_object::ptr&)> f) override;
	virtual void on_add(scene_object*);
	virtual void on_remove(scene_object*);
public:
	unsigned int rendered_simple;
	unsigned int rendered_instanced;
	unsigned int instances_count;

	Variable<bool> use_parrallel = Variable<bool>(true, "use_parrallel");
	Variable<bool> use_cpu_culling = Variable<bool>(true, "use_cpu_culling");
	Variable<bool> use_gpu_culling = Variable<bool>(false, "use_gpu_culling");
	Variable<bool> clear_depth = Variable<bool>(true, "clear_depth");


	Render::RootSignature::ptr my_signature;
	Render::StructuredBuffer<mat4x4>::ptr nodes_resource;

	using ptr = s_ptr<gpu_mesh_renderer>;
	gpu_mesh_renderer(Scene::ptr scene = nullptr);
};


template<class T,class ...Args>
class CyclicBuffer
{

	std::vector<T*> memory;
	std::list<int> removed;
public:

	void add(T* obj, Args...args)
	{
		memory.emplace_back(obj);
		set_index(obj, int(memory.size() - 1), args...);
	}

	void remove(int index , Args...args)
	{
		memory[index] = memory.back();
		memory.resize(memory.size() - 1);

		if (index<memory.size())
			set_index(memory[index], index, args...);
	}
	/*
	void remove_fast(int index)
	{


	memory.resize(memory.size() - 1);
	removed.insert(index);

	}

	void fix_fast_removed()
	{

	int counter = memory.size();
	for (auto index : removed)
	{
	memory[index] = memory[--counter];
	}

	set_index(memory[index], index);
	}
	*/

	//virtual void set_index(T *obj, int index) = 0;

	std::function<void(T*, int,Args...)> set_index;
};

template<class T>
class BufferMapper
{
	typename Render::StructuredBuffer<T>::ptr gpu_buffer;
	std::vector<T> cpu_buffer;
	bool changed = false;
public:
	void update_gpu(DX12::CommandList::ptr& list)
	{
		if (!gpu_buffer || gpu_buffer->get_count() < cpu_buffer.size())
			gpu_buffer = std::make_shared<Render::StructuredBuffer<T>>(cpu_buffer.size());
		gpu_buffer->set_data(list, 0, cpu_buffer);

		changed = false;
	}

	T& operator[](size_t i)
	{
		changed = true;
		return cpu_buffer[i];
	}

	void resize(size_t count)
	{
		cpu_buffer.resize(count);
	}

	size_t size()
	{
		return cpu_buffer.size();
	}
	
	typename	Render::StructuredBuffer<T>::ptr & get_gpu()
	{
		return gpu_buffer;
	}

	const std::vector<T> get_buffer() const {
		return cpu_buffer;
	}
};

template <class T>
class ArraysHolder:public std::vector<T> 
{

	

	std::map<size_t, size_t> free_sections_begins;
	std::map<size_t, size_t> free_sections_ends;

	std::map<size_t, std::set<size_t>> free_sections_sizes;


	std::map<size_t, size_t> used_sections;
	void remove_free_section(size_t index, size_t count)
	{
		free_sections_begins.erase(index);
		free_sections_ends.erase(index + count);
		free_sections_sizes[count].erase(index);
	}

	void add_free_section(size_t index, size_t count)
	{

		auto begins = free_sections_begins.find(index + count);

		if (begins != free_sections_begins.end())
		{
			count += begins->second;

			remove_free_section(begins->first, begins->second);
		}


		auto ends = free_sections_ends.find(index);

		if (ends != free_sections_ends.end())
		{
			index -= ends->second;
			remove_free_section(ends->first - ends->second, ends->second);
		}

		free_sections_begins[index] = count;
		free_sections_ends[index + count] = count;
		free_sections_sizes[count].insert(index);
	}
public:


	size_t get_free(size_t size)
	{
		auto res = free_sections_sizes.lower_bound(size);

		while (res != free_sections_sizes.end() && res->first < size)
			res++;

		if (res != free_sections_sizes.end()&& !res->second.empty())
		{
	
			auto index = *res->second.begin();

			res->second.erase(res->second.begin());


			auto free_section = free_sections_begins[index];
			remove_free_section(index, free_section);
			add_free_section(index + size, free_section - size);

			used_sections[index] = size;
	
			return index;
		}
		
		size_t result = std::vector<T>::size();
		resize(result + size);


		used_sections[result] = size;

		return result;

	}

	void release(size_t index)
	{

		add_free_section(index, used_sections[index]);
		used_sections.erase(index);
	}
};
class gpu_cached_renderer : public gpu_mesh_renderer,
	public Events::Runner
{

	virtual void on_add(scene_object*) override;
	virtual void on_remove(scene_object*) override;

	struct global_pipeline
	{
		std::map<size_t, pipeline_draws> pipeline_infos;
		std::mutex init_mutex;
	};

	void init_pipeline(global_pipeline&, Render::PipelineStateDesc, MaterialAsset*, std::tuple<RENDER_TYPE, MESH_TYPE, std::shared_ptr<materials::material>>);

	void init_material( MaterialAsset*);

	struct render_type_desc
	{
		std::map<Render::PipelineStateDesc, global_pipeline> pipelines;
		
	};
	std::map<std::tuple<RENDER_TYPE,MESH_TYPE, std::shared_ptr<materials::material> >, render_type_desc> type_pipelines;

	std::vector<size_t> indirection;
	std::map<size_t, mat_info> direction;


	struct material_data
	{
		int offset;
		Render::GPUBuffer::ptr ps_buffer;
	};

	ArraysHolder<Render::Handle> textures;
	std::map<MaterialAsset*, size_t> material_offsets;
	//std::vector<Render::Handle> textures;


	_voxel_info voxel_info;

	

	std::vector<MeshAssetInstance::render_info*> rendering_list;

	
	CyclicBuffer<MeshAssetInstance::render_info, MeshAssetInstance*> render_indices;
	CyclicBuffer<MeshAssetInstance::mesh_asset_node, MeshAssetInstance*> node_indices;


	BufferMapper<instance> boxes_instances;
	BufferMapper<IndirectCommand> gpu_commands;
	struct nodes
	{
		mat4x4 world;
		mat4x4 inv;
	};
	BufferMapper<nodes> gpu_nodes;

	std::mutex mtx;
	Render::ComputePipelineState::ptr gpu_frustum_pipeline[2];

	Render::StructuredBuffer<unsigned int>::ptr frustum_culled_commands;
	Render::StructuredBuffer<unsigned int>::ptr second_commands;
	bool rendering = false;

	bool updating_mats = false;
	MESH_TYPE mesh_type;
public:
	using ptr = std::shared_ptr<gpu_cached_renderer>;
	using 	DrawInfo = std::map<int, std::map<int, int>>;

	struct  
	{
	
			Events::Event<int> frustum_visible_commands;
		Events::Event<int> second_stage_test;
		Events::Event<DrawInfo> draw_infos;

		bool enabled = false;
	}statistic;
	void update(MeshRenderContext::ptr mesh_render_context);

	virtual void render(MeshRenderContext::ptr mesh_render_context, scene_object::ptr obj) override;
	void render_buffers(global_pipeline& pipeline, MeshRenderContext::ptr mesh_render_context, const Render::HandleTable& from_buffer);
	//using gpu_mesh_renderer::gpu_mesh_renderer;

	gpu_cached_renderer(Scene::ptr scene = nullptr, MESH_TYPE mesh_type = MESH_TYPE::ALL);
};


