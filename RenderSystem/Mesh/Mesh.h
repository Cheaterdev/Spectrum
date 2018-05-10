
struct mesh_data
{
        Render::vertex_mesh_data vertex_data;
        Render::indices_data index_data;
    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(vertex_data)&NVP(index_data);
        }
};

class SimpleMesh;
class Mesh;
class mesh_object;
class mesh_node;
class shared_mesh_object;
class node_object : public occluder, public std::enable_shared_from_this<node_object>
{
        friend class Mesh;
        friend class SimpleMesh;
        friend class mesh_object;
        friend class mesh_node;
        friend class shared_mesh_object;
        friend class mesh_renderer;
    public:
        typedef s_ptr<node_object> ptr;
        mat4x4 local_transform;
        mat4x4 mesh_transform;
        std::vector<ptr> childs;
        std::string name;
        std::vector<SimpleMesh*> meshes;
        virtual void add_child(ptr obj)
        {
            childs.push_back(obj);
        }
    private:

        node_object()
        {
            local_transform.identity();
            mesh_transform.identity();
        }

        virtual void calculate_size();



        void add_mesh(std::shared_ptr<SimpleMesh> mesh);

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar&  NVP(name)&NVP(local_transform)&NVP(mesh_transform) &NVP(childs)&NVP(boost::serialization::base_object<occluder>(*this));
        }
};

class SimpleMesh : public shared_object<SimpleMesh>
{
        friend class Mesh;
        friend class node_object;
        friend class mesh_renderer;

        Render::vertex_mesh vertexs;
        Render::index_mesh indices;
        mesh_data mesh_desc;

        std::vector<node_object::ptr> nodes;
        std::shared_ptr<Primitive> primitive;
//   DX11::vertex_shader::ptr v_shader;
        //  DX11::vertex_shader::ptr v_shader_instanced;
        unsigned int material_index;
        // Mesh* mesh;
    public:
        using ptr = s_ptr<SimpleMesh>;
        SimpleMesh(mesh_data data);
        void draw(MeshRenderContext::ptr context, unsigned int index);
        void draw_instanced(MeshRenderContext::ptr context, unsigned int index, unsigned int count);
    private:
        friend class boost::serialization::access;

        SimpleMesh();
        template<class Archive>
        void save(Archive& ar, const unsigned int) const
        {
            ar& NVP(mesh_desc)&NVP(nodes)&NVP(primitive)&NVP(material_index);
        }

        template<class Archive>
        void load(Archive& ar, const unsigned int)
        {
            ar& NVP(mesh_desc)&NVP(nodes)&NVP(primitive)&NVP(material_index);
            vertexs = Render::vertex_mesh(mesh_desc.vertex_data);
            indices = Render::index_mesh(mesh_desc.index_data);

            for (auto node : nodes)
                node->meshes.push_back(this);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()

};
/*
class Mesh : public node_object, public std::enable_shared_from_this<Mesh>, public shared_object<Mesh>, public AssetHolder
{
        friend class resource_manager_simple<Mesh, std::string>;
        friend class resource_manager<Mesh, std::string>;
        friend class loader<Mesh, std::string>;
        friend class mesh_renderer;
        friend class mesh_object;
        friend class shared_mesh_object;

        typedef std::vector<SimpleMesh::ptr> meshes_type;

        //   static std::shared_ptr<Mesh> load_assimp(const std::string& file_name, resource_file_depender& files);
        //   static bool init_default_loaders();
    public:
        using ptr = s_ptr<Mesh>;
        std::vector<MaterialAsset::ref> materials;
        meshes_type meshes;
        Mesh(meshes_type data, std::vector<MaterialAsset::ptr> materials);
    private:
        Mesh() = default;
        Mesh(const Mesh&) = default;
        Mesh& operator=(const Mesh&) = default;

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(materials);
            ar& NVP(meshes)&NVP(boost::serialization::base_object<node_object>(*this))&NVP(boost::serialization::base_object<AssetHolder>(*this));
        }
};
*/
class material_holder
{

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
        }

    public:
        virtual void set(MeshRenderContext::ptr context) = 0;
        virtual ~material_holder() {}
};
class mesh_renderer;
class mesh_node : public scene_object, public Render::renderable
{
        friend class mesh_object;
        friend class mesh_renderer;
        material_holder* mesh;
        //      Render::ConstBuffer<mat4x4>::ptr const_buffer;
        std::vector<SimpleMesh*> meshes;

    protected:
     //   bool update_transforms() override;
    public:
        using ptr = s_ptr<mesh_node>;

        mesh_node(std::shared_ptr<node_object> node, material_holder* mesh);

        void draw(MeshRenderContext::ptr context);

    private:
        mesh_node()
        {
            //   if (meshes.size())
            //   const_buffer.reset(new Render::ConstBuffer<mat4x4>());
        };
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(meshes)&NVP(boost::serialization::base_object<scene_object>(*this));
        }
};
/*
class mesh_object : public scene_object, public material_holder
{
        std::shared_ptr<Mesh> mesh;
    public:
        std::vector<MaterialAsset::ref> materials;
        using ptr = s_ptr<mesh_object>;
        mesh_object(std::shared_ptr<Mesh> mesh);

        virtual void set(MeshRenderContext::ptr context) override;

};

*/
/*
class shared_mesh_object: public scene_object, public Render::renderable, public shared_object<Mesh>::owner
{
        friend class mesh_renderer;
        std::shared_ptr<Mesh> mesh;
        std::vector<MaterialAsset::ref> materials;
    public:
        using ptr = s_ptr<shared_mesh_object>;
        shared_mesh_object(std::shared_ptr<Mesh> mesh)
        {
            this->mesh = mesh;
            materials = mesh->materials;
            set_local_transform(mesh->local_transform);

//            if (mesh->primitive)
            {
          //      primitive = mesh->primitive->clone();
           //     childs_occluder.primitive = primitive->clone();
            }

            shared_object<Mesh>::owner::add_shared(mesh.get());
        }

        virtual void draw(MeshRenderContext::ptr context);
        virtual bool is_inside(const Frustum& f)
        {

			return false;

        }


};
*/
BOOST_CLASS_EXPORT_KEY(node_object);
BOOST_CLASS_EXPORT_KEY(mesh_node);
BOOST_CLASS_EXPORT_KEY(Mesh);
BOOST_CLASS_EXPORT_KEY(material_holder);
