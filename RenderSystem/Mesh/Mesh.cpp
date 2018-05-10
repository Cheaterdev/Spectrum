#include "pch.h"
//boost exports

BOOST_CLASS_EXPORT_IMPLEMENT(node_object);
//BOOST_CLASS_EXPORT_IMPLEMENT(Mesh);
BOOST_CLASS_EXPORT_IMPLEMENT(mesh_node);
BOOST_CLASS_EXPORT_IMPLEMENT(material_holder);

template void mesh_node::serialize(serialization_oarchive& arch, const unsigned int version);
template void mesh_node::serialize(serialization_iarchive& arch, const unsigned int version);

template void material_holder::serialize(serialization_oarchive& arch, const unsigned int version);
template void material_holder::serialize(serialization_iarchive& arch, const unsigned int version);



//////////////////////////////////////////////////////////////////////////

void node_object::add_mesh(SimpleMesh::ptr mesh)
{
/*    if (!primitive)
        primitive.reset(new AABB(mesh->primitive.get()));
    else
        primitive->combine(mesh->primitive.get());
		*/
    mesh->nodes.push_back(std::dynamic_pointer_cast<node_object>(shared_from_this()));
    meshes.push_back(mesh.get());
}

void node_object::calculate_size()
{
    for (auto& c : childs)
        c->calculate_size();

   /* if (meshes.size())
    {
        if (!primitive)
            primitive.reset(new AABB((*meshes.begin())->primitive.get()));

        for (auto& m : meshes)
            ((AABB*)primitive.get())->combine(m->primitive.get());
    }

    if (childs.size())
    {
        if (!primitive)
            primitive.reset(new AABB((*childs.begin())->primitive.get()));

        for (auto& c : childs)
            primitive->combine(c->primitive.get(), c->local_transform);
    }

    if (!primitive)
        primitive.reset(new AABB());*/
}

SimpleMesh::SimpleMesh()
{
    // v_shader = DX11::vertex_shader::get_resource({ "triangle.hlsl", "VS", 0, {} });
    // v_shader_instanced = DX11::vertex_shader::get_resource({ "triangle.hlsl", "VS_instanced", 0, {} });
}

SimpleMesh::SimpleMesh(mesh_data data) : vertexs(data.vertex_data), indices(data.index_data), mesh_desc(data)
{
    //  v_shader = DX11::vertex_shader::get_resource({ "triangle.hlsl", "VS", 0, {} });
    //  v_shader_instanced = DX11::vertex_shader::get_resource({ "triangle.hlsl", "VS_instanced", 0, {} });
}

void SimpleMesh::draw(MeshRenderContext::ptr context, unsigned int index)
{
    context->use_material(material_index, context);//[material_index]->set(context);
    context->transformer->set(context, vertex_types::simple);
    context->list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    vertexs.set(context);
    indices.set(context);
    auto& mesh = mesh_desc.index_data.meshes[index];
    context->draw_indexed(mesh.size, mesh.offset, mesh.vertex_offset);
}

void SimpleMesh::draw_instanced(MeshRenderContext::ptr context, unsigned int index, unsigned int count)
{
    /*	context.materials[material_index]->set(context);
    	context.transformer->set(context, vertex_types::instanced);
        context.dx_context.set_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    	vertexs.set(context.dx_context, context.dx_context.get_vertex_shader());
      //  context.dx_context.set(v_shader_instanced);
        indices.set(context.dx_context);
        auto &mesh = mesh_desc.index_data.meshes[index];
        context.dx_context.draw_indexed_instanced(mesh.size, mesh.offset, mesh.vertex_offset, count, 0);*/
}
/*
Mesh::Mesh(meshes_type data, std::vector<MaterialAsset::ptr> materials) : Mesh()
{
    for (auto& m : materials)
        this->materials.push_back(register_asset(m));

    meshes = data;
}*/
/*
bool Mesh::init_default_loaders()
{
    add_loader(load_assimp);
    return true;
}
*/
//////////////////////////////////////////////////////////////////////////

void mesh_node::draw(MeshRenderContext::ptr context)
{
    mesh->set(context);

    for (auto& mesh : meshes)
    {
        //assert(const_buffer != nullptr);
        // context->set_mesh_const_buffer(const_buffer);
        //  context->set_mesh_const_buffer(global_transform);
//        context.dx_context.get_shader_state<DX11::vertex_shader>().const_buffer_state[1] = const_buffer;              HERE
        mesh->draw(context, 0);
    }
}

mesh_node::mesh_node(std::shared_ptr<node_object> node, material_holder* mesh)
{
    ownered = true;
    this->mesh = mesh;
    set_local_transform(node->local_transform);
    meshes = node->meshes;
    name = convert(node->name);
/*
    if (node->primitive)
        primitive = node->primitive->clone();
	*/
    //  if *(meshes.size())
    //      const_buffer.reset(new Render::ConstBuffer<mat4x4>());

    for (auto c : node->childs)
        add_child(ptr(new mesh_node(c, mesh)));
}
/*
void mesh_node::update_transforms()
{
    scene_object::update_transforms();
//   if (const_buffer)
//       (*const_buffer)[0] = global_transform;
}*/

/*
mesh_object::mesh_object(std::shared_ptr<Mesh> mesh)
{
    this->mesh = mesh;
    name = convert(mesh->name);
    materials = mesh->materials;
    set_local_transform(mesh->local_transform);
	
	
    for (auto c : mesh->childs)
        add_child(scene_object::ptr(new mesh_node(c, this)));
}

void mesh_object::set(MeshRenderContext::ptr context)
{
//    context->materials = materials;
}

void shared_mesh_object::draw(MeshRenderContext::ptr context)
{
    //  context->materials = materials;
}
*/