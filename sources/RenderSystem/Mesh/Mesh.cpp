#include "pch.h"
//boost exports

BOOST_CLASS_EXPORT_IMPLEMENT(node_object);
//BOOST_CLASS_EXPORT_IMPLEMENT(Mesh);
BOOST_CLASS_EXPORT_IMPLEMENT(mesh_node);


template void mesh_node::serialize(serialization_oarchive& arch, const unsigned int version);
template void mesh_node::serialize(serialization_iarchive& arch, const unsigned int version);

//////////////////////////////////////////////////////////////////////////

void node_object::add_mesh(SimpleMesh::ptr mesh)
{

    mesh->nodes.push_back(std::dynamic_pointer_cast<node_object>(shared_from_this()));
    meshes.push_back(mesh.get());
}

void node_object::calculate_size()
{
    for (auto& c : childs)
        c->calculate_size();

 
}

SimpleMesh::SimpleMesh()
{

}

SimpleMesh::SimpleMesh(mesh_data data) : vertexs(data.vertex_data), indices(data.index_data), mesh_desc(data)
{

}


mesh_node::mesh_node(std::shared_ptr<node_object> node)
{
    ownered = true;

    set_local_transform(node->local_transform);
    meshes = node->meshes;
    name = convert(node->name);

    for (auto c : node->childs)
        add_child(ptr(new mesh_node(c)));
}