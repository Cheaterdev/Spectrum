#include "Vertexes.h"
#include "Occlusion/Occluder.h"
#include "patterns/SharedObject.h"
#include "Scene/SceneObject.h"

struct mesh_data
{
        DX12::vertex_mesh_data vertex_data;
        DX12::indices_data index_data;
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

        DX12::vertex_mesh vertexs;
        DX12::index_mesh indices;
        mesh_data mesh_desc;

        std::vector<node_object::ptr> nodes;
        std::shared_ptr<Primitive> primitive;

        unsigned int material_index;

    public:
        using ptr = s_ptr<SimpleMesh>;
        SimpleMesh(mesh_data data);
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
            vertexs = DX12::vertex_mesh(mesh_desc.vertex_data);
            indices = DX12::index_mesh(mesh_desc.index_data);

            for (auto node : nodes)
                node->meshes.push_back(this);
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()

};


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
class mesh_node : public scene_object, public DX12::renderable
{
        friend class mesh_object;
        friend class mesh_renderer;
        material_holder* mesh;
        std::vector<SimpleMesh*> meshes;

    public:
        using ptr = s_ptr<mesh_node>;

        mesh_node(std::shared_ptr<node_object> node, material_holder* mesh);

    private:
        mesh_node() = default;
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(meshes)&NVP(boost::serialization::base_object<scene_object>(*this));
        }
};

BOOST_CLASS_EXPORT_KEY(node_object);
BOOST_CLASS_EXPORT_KEY(mesh_node);
BOOST_CLASS_EXPORT_KEY(Mesh);
BOOST_CLASS_EXPORT_KEY(material_holder);
