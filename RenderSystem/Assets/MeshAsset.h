class MeshAssetInstance;

struct Vertex
{
    float3 pos;
    float3 normal;
    float2 tc;
    float4 tangent;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& NVP(pos);
		ar& NVP(normal);
		ar& NVP(tc);
		ar& NVP(tangent);
	}

};

struct MeshInfo
{
        unsigned int vertex_offset;
        unsigned int vertex_count;
        size_t material;

        unsigned int index_offset;
        unsigned int index_count;
        std::shared_ptr<Primitive> primitive;
		
        unsigned int node_index;
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(vertex_offset);
            ar& NVP(vertex_count);
            ar& NVP(material);
            ar& NVP(index_offset);
            ar& NVP(index_count);
            ar& NVP(primitive);
            ar& NVP(node_index);
	//		vertex_offset = 0;
        }
};

using command = Table::CommandData::CB;
using mesh_info_part = Table::MeshCommandData::CB;
using material_info_part = Table::MaterialCommandData::CB;


struct MeshNode
{
        mat4x4 local_matrix;
        mat4x4 mesh_matrix;
        using ptr = s_ptr<MeshNode>;
        std::vector<unsigned int> meshes;
        std::vector<MeshNode> childs;
        unsigned int index;
	
        virtual void iterate(std::function<bool(MeshNode*)> f)
        {
            if (f(this))
            {
                for (auto& c : childs)
                    c.iterate(f);
            }
        }

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(local_matrix);
            ar& NVP(mesh_matrix);
            ar& NVP(meshes);
            ar& NVP(childs);
//           ar& NVP(index);
        }
};

class AssetLoadingContext
{
public:

	using ptr = std::shared_ptr<AssetLoadingContext>;

	GUI::user_interface::ptr ui;
	Task::ptr loading_task;


};
class MeshData: public loader<MeshData, std::string, AssetLoadingContext::ptr>//, public resource_manager_simple<MeshData, std::string>
{
    public:
        using ptr = std::shared_ptr<MeshData>;
        std::vector<Vertex> vertex_buffer;
        std::vector<UINT32> index_buffer;

        std::vector<MeshInfo> meshes;
        std::vector<MaterialAsset::ptr> materials;
        //std::vector<MeshNode*> nodes;
        MeshNode root_node;
        std::shared_ptr<Primitive> primitive;

        static std::shared_ptr<MeshData> load_assimp(const std::string& file_name, resource_file_depender& files, AssetLoadingContext::ptr & context);
        static bool init_default_loaders();
		virtual void calculate_size();
};
class MeshAssetInstance;
class MeshAsset : public Asset
{
        LEAK_TEST(MeshAsset)

        typedef std::vector<SimpleMesh::ptr> meshes_type;

        MeshAsset();
        std::shared_ptr<MeshAssetInstance> preview_mesh;
    public:
		static const Asset_Type TYPE = Asset_Type::MESH;

        using ptr = s_ptr<MeshAsset>;
        using ref = AssetReference<MeshAsset>;
	
        std::vector<Vertex> vertex_buffer;
        std::vector<UINT32> index_buffer;

        TypedHandle<Vertex> vertex_handle;
        TypedHandle<UINT32> index_handle;



        std::vector<MeshInfo> meshes;
        std::vector<MaterialAsset::ref> materials;

        std::shared_ptr<Primitive> primitive;
        mat4x4 local_transform;
        MeshNode root_node;
        std::vector<MeshNode*> nodes;

        virtual void shutdown() override
        {
            preview_mesh.reset();
        }
        MeshAsset(std::wstring, AssetLoadingContext::ptr =nullptr);
        virtual ~MeshAsset()
        {
            materials.clear();
        }
        std::shared_ptr<MeshAssetInstance> create_instance();
        virtual void try_register();

        virtual Asset_Type get_type();
        virtual void update_preview(Render::Texture::ptr preview);
        virtual AssetStorage::ptr  register_new(std::wstring name = L"", Guid g = Guid());
        virtual void reload_resource() override
        {
            // mesh->reload_resource();
        }


        void init_gpu();
    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            // ar& NVP(local_transform);
            ar& NVP(primitive);
            //   ar& NVP(meshes);
            //   ar& NVP(materials);
            ar& NVP(vertex_buffer);
            ar& NVP(index_buffer);
            ar& NVP(meshes);
            ar& NVP(materials);
            ar& NVP(local_transform);
            ar& NVP(root_node);
            ar& NVP(nodes);
            ar& NVP(boost::serialization::base_object<Asset>(*this));

            if constexpr (Archive::is_loading::value)
            {
                init_gpu();
            }
        }

};

class MeshAssetInstance : public scene_object, public material_holder, public AssetHolder, public MaterialProvider, public Render::renderable
{
        friend class mesh_renderer;
		friend class gpu_mesh_renderer;
		friend class gpu_cached_renderer;


        friend class stencil_renderer;

     
        LEAK_TEST(MeshAssetInstance)

        virtual void set(MeshRenderContext::ptr context) override;
        std::vector<mesh_node::ptr> my_meshes;
        std::mutex m;
        void init_asset();
		void update_nodes();



		std::vector<RaytracingAccelerationStructure::ptr> raytracing_as;
     
        TypedHandle<mesh_info_part> meshpart_handle;

        size_t nodes_count;
        size_t rendering_count;
    public:
		MESH_TYPE type= MESH_TYPE::STATIC;
		MeshAsset::ref mesh_asset;

		struct mesh_asset_node
		{
			MeshNode* asset_node;
			unsigned int global_index;
			mesh_asset_node(MeshNode* node)
			{
				asset_node = node;
			}
		};


        TypedHandle<Table::node_data::CB> nodes_handle;
        struct render_info
        {
  
			std::shared_ptr<Primitive> primitive;
            std::shared_ptr<Primitive> primitive_global;
            Slots::MeshInfo mesh_info;

            D3D12_DRAW_INDEXED_ARGUMENTS draw_arguments;
            UINT material_id;
            MaterialAsset* material;


            //compiled
			Slots::MeshInfo::Compiled compiled_mesh_info;
                

        };

        std::vector<render_info> rendering;
        std::vector<mesh_asset_node> nodes;
		std::vector<int> nodes_indexes;

        std::vector<MaterialAsset::ref> overrided_material;



		std::vector<RaytracingAccelerationStructure::ptr> create_raytracing_as(D3D12_GPU_VIRTUAL_ADDRESS);
	
        void use_material(size_t i, std::shared_ptr<MeshRenderContext>& context) override;

        using ptr = s_ptr<MeshAssetInstance>;
        virtual ~MeshAssetInstance();
        MeshAssetInstance(MeshAsset::ptr asset);
        void draw(MeshRenderContext::ptr context);

        void override_material(size_t i, MaterialAsset::ptr mat);

		virtual void on_asset_change(Asset::ptr asset);
      
	
    virtual void on_add(scene_object * parent) override;

	virtual void on_remove() override;
	virtual	bool update_transforms()override;
    private:
        void debug_draw(debug_drawer& drawer) override
        {
            scene_object::debug_draw(drawer);
	//		drawer.add(primitive, global_transform);
			mat4x4 m;
			m.identity();
            for (auto &r : rendering)
            {
				

                //	r.primitive->
                drawer.add(r.primitive_global, m);
            }
        }
        MeshAssetInstance();

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int);

};



class universal_nodes_manager :public Singleton<universal_nodes_manager>, public virtual_gpu_buffer<Table::node_data::CB>
{
    static const int MAX_NODES_SIZE = 1_gb / sizeof(Table::node_data::CB);
public:
    universal_nodes_manager():virtual_gpu_buffer<Table::node_data::CB>(MAX_NODES_SIZE)
	{
      
    }
};

class universal_vertex_manager :public Singleton<universal_vertex_manager>, public virtual_gpu_buffer<Vertex>
{
	static const int MAX_VERTEXES_SIZE = 1_gb / sizeof(Vertex);
public:
    universal_vertex_manager() :virtual_gpu_buffer<Vertex>(MAX_VERTEXES_SIZE)
	{

	}
};

class universal_index_manager :public Singleton<universal_index_manager>, public virtual_gpu_buffer<UINT32>
{
	static const int MAX_INDEX_SIZE = 1_gb / sizeof(UINT32);
public:
    universal_index_manager() :virtual_gpu_buffer<UINT32>(MAX_INDEX_SIZE)
	{

	}
};

class universal_material_manager :public Singleton<universal_material_manager>, public virtual_gpu_buffer<std::byte>
{
    static const int MAX_bytes_SIZE = 1_gb;
public:
    universal_material_manager() :virtual_gpu_buffer<std::byte>(MAX_bytes_SIZE)
	{

	}
};



class universal_material_info_part_manager :public Singleton<universal_material_info_part_manager>, public virtual_gpu_buffer<material_info_part>
{
	static const int MAX_COMMANDS_SIZE = 1_gb/sizeof(material_info_part);
public:
    universal_material_info_part_manager() :virtual_gpu_buffer<material_info_part>(MAX_COMMANDS_SIZE)
	{

	}
};

class SceneFrameManager :public Singleton<SceneFrameManager>
{

public:
    void prepare(CommandList::ptr& command_list, Scene & scene);
};







//BOOST_CLASS_TRACKING(material_holder, boost::serialization::track_never);
BOOST_CLASS_EXPORT_KEY(MeshAssetInstance);
BOOST_CLASS_EXPORT_KEY(MeshAsset);
BOOST_CLASS_EXPORT_KEY(AssetReference<MeshAsset>);
