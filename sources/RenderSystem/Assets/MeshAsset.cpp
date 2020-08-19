#include "pch.h"




bool MeshData::init_default_loaders()
{
    add_loader(load_assimp);
    return true;
}
 void MeshData::calculate_size()
{
	primitive.reset(new AABB());
	bool first = true;
	std::function<bool(MeshNode*)> f = [&first, this](MeshNode * r)->bool
	{

		//for (auto m : r->meshes)

		if(r->mesh_id!=-1)
		{
			auto p = meshes[r->mesh_id].primitive;
			assert(p);
			/*	if (primitive)
			((AABB*)childs_occluder.primitive.get())->set(primitive.get());
			else
			((AABB*)childs_occluder.primitive.get())->set((*childs.begin())->childs_occluder.primitive.get());

			for (auto& c : childs)*/

			if (first)
				primitive->set(p.get(), r->mesh_matrix);
			else
				primitive->combine(p.get(), r->mesh_matrix);

			first = false;
		}

		return true;
	};
	root_node.iterate(f);
}
/*

class TextureAssetRenderer : public Singleton<TextureAssetRenderer>
{
	friend class Singleton<TextureAssetRenderer>;

	DX11::pipeline_state::ptr state;
	DX11::sampler_state::ptr sampler;
	std::vector<D3D11_VIEWPORT> vps;

	TextureAssetRenderer()
	{
		state = DX11::pipeline_state::create();
		state->p_shader = DX11::pixel_shader::get_resource({ "shaders\\texture_drawer.hlsl", "PS", 0, {} });
		state->v_shader = DX11::vertex_shader::get_resource({ "shaders\\texture_drawer.hlsl", "VS", 0, {} });
		D3D11_SAMPLER_DESC sampDesc;
		sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.MipLODBias = 0.0f;
		sampDesc.MaxAnisotropy = 16;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		sampler = DX11::sampler_state::get_resource(sampDesc);
		D3D11_BLEND_DESC b_desc;
		ZeroMemory(&b_desc, sizeof(b_desc));

		for (int i = 0; i < 4; ++i)
		{
			b_desc.RenderTarget[i].BlendEnable = FALSE;
			b_desc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			b_desc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			b_desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			b_desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			b_desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			b_desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			b_desc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}

		state->blend = DX11::blend_state::get_resource(b_desc);
		D3D11_DEPTH_STENCIL_DESC depth_desc;
		depth_desc.DepthEnable = false;
		depth_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depth_desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		depth_desc.StencilEnable = false;
		depth_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		depth_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		depth_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depth_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depth_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depth_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depth_desc.BackFace = depth_desc.FrontFace;
		state->depth_stencil = DX11::depth_stencil_state::get_resource(depth_desc);
		state->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		D3D11_RASTERIZER_DESC raster_desc;
		ZeroMemory(&raster_desc, sizeof(raster_desc));
		raster_desc.CullMode = D3D11_CULL_NONE;
		raster_desc.FillMode = D3D11_FILL_SOLID;
		raster_desc.ScissorEnable = false;
		state->rasterizer = DX11::rasterizer_state::get_resource(raster_desc);
		vps.resize(1);
		vps[0].TopLeftX = 0;
		vps[0].TopLeftY = 0;
		vps[0].MinDepth = 0.0f;
		vps[0].MaxDepth = 1.0f;
	}
	virtual ~TextureAssetRenderer() {}
public:
	void render(TextureAsset* asset, DX11::Texture2D::ptr target, DX11::Context &c)
	{
		auto orig = c.get_rts();
		auto orig_vps = c.get_viewports();
		DX11::RenderTargetState my_state;
		vps[0].Width = target->get_desc().Width;
		vps[0].Height = target->get_desc().Height;
		c.clear_rtv(target->get_render_target(), { 0, 0, 0, 0 });
		my_state[0] = target->get_render_target();
		c.get_rts() = my_state;
		c.get_shader_state< DX11::pixel_shader>().sampler_state[0] = sampler;
		c.get_shader_state< DX11::pixel_shader>().shader_resource_state[0] = asset->get_texture()->get_shader_view();
		c.set(state);
		c.set_viewports(vps);
		c.draw(4, 0);
		c.get_rts() = orig;
		c.set_viewports(orig_vps);
		c.flush();
	}
};

*/
//////////////////////////////////////////////////////////////////////////
Asset_Type MeshAsset::get_type()
{
    return Asset_Type::MESH;
}


void MeshAsset::init_gpu()
{

	{
		universal_vertex_manager::get().allocate(vertex_handle, vertex_buffer.size());
		auto vert = vertex_buffer;
		vertex_handle.write(0, vert);
	}

	{
		 universal_index_manager::get().allocate(index_handle, index_buffer.size());
		auto vert = index_buffer;
		index_handle.write(0, vert);
	}
}
MeshAsset::MeshAsset(std::wstring file_name, AssetLoadingContext::ptr c)
{
    auto task = TaskInfoManager::get().create_task(file_name);
	if (!c) c = std::make_shared<AssetLoadingContext>();
	 c->loading_task = task;
    auto data = MeshData::get_resource(convert(file_name),c);

	 c->loading_task = nullptr;
    if (!data) return;

    vertex_buffer = data->vertex_buffer;
	index_buffer = data->index_buffer;

	init_gpu();

    meshes = data->meshes;
    root_node = data->root_node;
    local_transform.identity();
    root_node.iterate([this](MeshNode * node)
    {
        nodes.push_back(node);
        //	for(auto c:childs)
        return true;
    });

    for (auto& m : data->materials)
        materials.emplace_back(register_asset(m));

    primitive = data->primitive->clone();
    /*(AABB*)primitive.get())->min = { -5, -5, -5 };
    ((AABB*)primitive.get())->max = { 5, 5, 5 };*/
    name = file_name.substr(file_name.find_last_of(L"\\") + 1);
    mark_changed();
}
AssetStorage::ptr  MeshAsset::register_new(std::wstring name, Guid g)
{
    if (meshes.size())
       return  Asset::register_new(name, g);

	return nullptr;
}

MeshAsset::MeshAsset()
{
}

std::shared_ptr<MeshAssetInstance> MeshAsset::create_instance()
{
	return std::make_shared<MeshAssetInstance>(get_ptr<MeshAsset>());
}

void MeshAsset::try_register()
{
    if (meshes.size())
        Asset::try_register();
}


void MeshAsset::update_preview(Render::Texture::ptr preview)
{
    if (!preview || !preview->is_rt())
        preview.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 256, 256, 1, 6, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));

    if (!preview_mesh)
        preview_mesh.reset(new MeshAssetInstance(get_ptr<MeshAsset>()));

    AssetRenderer::get().draw(preview_mesh, preview);
    /*
    TextureAssetRenderer::get().render(this, preview, c);
    c.generate_mipmaps(preview->get_shader_view());
    mark_changed();*/
    // c.generate_mipmaps(preview->get_shader_view());
    mark_changed();
}
void MeshAssetInstance::use_material(size_t i, std::shared_ptr<MeshRenderContext>& context)
{
    rendering[i].material->set(this->type,context);
}

void MeshAssetInstance::set(MeshRenderContext::ptr context)
{
    // context->materials.resize(overrided_material.size());
    /*	if (!overrided_material)
    	for (int i = 0; i < mesh_asset->get_mesh()->materials.size(); i++)
    		context.materials[i] = mesh_asset->get_mesh()->materials[i]->get_material();
    	else*/
    // for (int i = 0; i < overrided_material.size(); i++)
    ///     context->materials[i] = overrided_material[i]->get_material();
    // std::vector<materials::material::ptr> mats;
    // mats.resize(overrided_material.size());
    // for (int i = 0; i < overrided_material.size(); i++)
    //     mats[i] = overrided_material[i]->get_material();
    context->mat_provider = this;//(mats);
    //= mesh_asset->get_mesh()->materials;
}

template<class Archive>
void MeshAssetInstance::serialize(Archive& ar, const unsigned int)
{
    ar& NVP(boost::serialization::base_object<AssetHolder>(*this));
	ar& NVP(overrided_material);
    ar& NVP(mesh_asset);
    ar& NVP(boost::serialization::base_object<scene_object>(*this));
    ar& NVP(my_meshes);
	ar& NVP(type);

    if (Archive::is_loading::value)
        init_asset();
}

MeshAssetInstance::MeshAssetInstance(MeshAsset::ptr asset) : mesh_asset(this)
{
    mesh_asset = register_asset(asset);
   this->overrided_material.clear();// = mesh_asset->get_mesh()->materials;

    for (auto& m : mesh_asset->materials)
        this->overrided_material.push_back(register_asset(*m));
		
    init_asset();
    name = asset->get_name();
}

MeshAssetInstance::MeshAssetInstance(): mesh_asset(this)
{
}

void MeshAssetInstance::override_material(size_t i, MaterialAsset::ptr mat)
{

	auto& info = rendering[i];
	Scene* render_scene = dynamic_cast<Scene*>(scene);

	auto meshpart = meshpart_handle.map(i);//  render_scene->mesh_infos->map_elements(meshpart_handle.get_offset(), rendering_count);

	overrided_material[info.material_id] = register_asset(mat);
	rendering[i].material = overrided_material[info.material_id]->get_ptr<MaterialAsset>().get();

	meshpart[0].material_id = static_cast<materials::universal_material*>(rendering[i].material)->get_material_id();
	meshpart[0].mesh_cb = info.compiled_mesh_info.cb;
	meshpart[0].draw_commands = info.draw_arguments;

	meshpart_handle.write(i, meshpart);

	if (scene)
		scene->on_changed(this);
}

void MeshAssetInstance::on_asset_change(Asset::ptr asset)
{
	if (asset->get_type() == Asset_Type::MATERIAL)
	{

		std::vector<MaterialAsset::ptr> changed;
		changed.reserve(overrided_material.size());

		auto material = asset->get_ptr<MaterialAsset>();
		for (auto& ref : overrided_material)
		{

			if (ref->get_ptr<MaterialAsset>() == material)
				changed.push_back(material);
			else
				changed.push_back(nullptr);
		}
	}
	if (scene)
		scene->on_changed(this);
}

MeshAssetInstance::~MeshAssetInstance()
{
}
void MeshAssetInstance::on_add(scene_object* parent)
{

	auto old_scene = scene;
	scene_object::on_add(parent);

	if (!old_scene&&scene)
	{

		Scene* render_scene = dynamic_cast<Scene*>(scene);

		render_scene->mesh_infos->allocate(meshpart_handle, rendering_count);
		auto meshpart = meshpart_handle.map();//  render_scene->mesh_infos->map_elements(meshpart_handle.get_offset(), rendering_count);


		int i = 0;
		for (auto& info  : rendering)
		{

			meshpart[i].mesh_cb = info.compiled_mesh_info.cb;
			meshpart[i].material_id = static_cast<materials::universal_material*>(info.material)->get_material_id();
			meshpart[i].draw_commands = info.draw_arguments;
			meshpart[i].node_offset = info.mesh_info.GetNode_offset();
	

			render_scene->command_ids[int(type)].insert(meshpart_handle.get_offset() + i);
			render_scene->command_ids[int(MESH_TYPE::ALL)].insert(meshpart_handle.get_offset() + i);

			i++;
		}

		meshpart_handle.write(0, meshpart);
	}
}


void MeshAssetInstance::on_remove()
{
	auto old_scene = scene;
	scene_object::on_remove();

	if (old_scene&&!scene)
	{
		int i = 0;
		for (auto& info : rendering)
		{
			old_scene->command_ids[int(type)].erase(meshpart_handle.get_offset() + i);
			old_scene->command_ids[int(MESH_TYPE::ALL)].erase(meshpart_handle.get_offset() + i);
			i++;
		}
		meshpart_handle.Free();
		meshpart_handle = TypedHandle<mesh_info_part>();
	}
}
bool MeshAssetInstance::update_transforms()
{
	bool res = scene_object::update_transforms();

	if (res)
	{

//		node_buffer.data().clear();
//	node_buffer.data().reserve(mesh_asset->nodes.size());
	//	nodes.clear();


		auto gpu_nodes = nodes_handle.map();// universal_nodes_manager::get().map_elements(nodes_handle.get_offset(), nodes_count);

	for (auto & info : rendering)
	{
		uint index = info.mesh_info.GetNode_offset() - nodes_handle.get_offset();
		auto& p = info.primitive;

		if (!info.primitive_global)
			info.primitive_global = std::make_shared<AABB>();


		mat4x4 mat = nodes[index].asset_node->mesh_matrix*global_transform;
		info.primitive_global->apply_transform(p, mat);

		auto &my_node = gpu_nodes[info.mesh_info.GetNode_offset() - nodes_handle.get_offset()];
		my_node.node_global_matrix = mat;
		my_node.node_inverse_matrix = mat;
		my_node.node_inverse_matrix.inverse();


		my_node.aabb.min = info.primitive->get_min();
		my_node.aabb.max = info.primitive->get_max();


	}

	nodes_handle.write(0, gpu_nodes);
	if (scene)
		scene->on_moved(this);
}
return res;
}


void MeshAssetInstance::update_nodes()
{
//	primitive.reset(new AABB());
	
	nodes_count = 0;
	rendering_count = 0;

	mesh_asset->root_node.iterate([&](MeshNode* m) {nodes_count++; rendering_count += (m->mesh_id!=-1); return true; });

	
	 universal_nodes_manager::get().allocate(nodes_handle, nodes_count);

	auto gpu_nodes = nodes_handle.map();// universal_nodes_manager::get().map_elements(nodes_handle.get_offset(), nodes_count);

//	nodes_ptr = universal_nodes_manager::get().nodes_data.data() + nodes_handle.aligned_offset;

	//	mesh_render_data.GetNodes() = render_scene->mesh_nodes->get_srv()[0];

	/////////////////

  // node_buffer.clear();
  //  node_buffer.reserve(mesh_asset->nodes.size());
	nodes_indexes.resize(mesh_asset->nodes.size());
    nodes.clear();
    rendering.clear();

	
    std::function<bool(MeshNode*)> f = [&](MeshNode * node)->bool
    {

		if (node->mesh_id == -1) return true;
		int m = node->mesh_id;
	//	for (auto& m : node->meshes)
		{
nodes.emplace_back(node);

			

			render_info info;
			info.draw_arguments.StartIndexLocation = mesh_asset->index_handle.get_offset() + mesh_asset->meshes[m].index_offset;
			info.draw_arguments.IndexCountPerInstance = mesh_asset->meshes[m].index_count;
			info.draw_arguments.BaseVertexLocation = 0;
			info.draw_arguments.InstanceCount = 1;
			info.draw_arguments.StartInstanceLocation = 0;

			info.primitive = mesh_asset->meshes[m].primitive;

			info.primitive_global = std::make_shared<AABB>();

			mat4x4 mat = nodes.back().asset_node->mesh_matrix * global_transform;

			info.primitive_global->apply_transform(info.primitive, mat);


		//	node_buffer[node->index] = mat;
			//if(nodes_ptr) 
		
			info.mesh_info.GetNode_offset() = nodes_handle.get_offset() + nodes.size()-1;
			info.mesh_info.GetVertex_offset() = mesh_asset->vertex_handle.get_offset() + mesh_asset->meshes[m].vertex_offset;

			auto &my_node = gpu_nodes[info.mesh_info.GetNode_offset() - nodes_handle.get_offset()];
			my_node.node_global_matrix = mat;
			my_node.node_inverse_matrix = mat;
			my_node.node_inverse_matrix.inverse();


		//	my_node.aabb.min = info.primitive->get_min();
		//	my_node.aabb.max = info.primitive->get_max();

			info.material_id = mesh_asset->meshes[m].material;
			info.material = overrided_material[info.material_id]->get_ptr<MaterialAsset>().get();
			info.compiled_mesh_info = info.mesh_info.compile(StaticCompiledGPUData::get());

			rendering.push_back(info);
		}


        return true;
    };
	
    mesh_asset->root_node.iterate(f);

	nodes_handle.write(0, gpu_nodes);
}


void MeshAssetInstance::init_asset()
{
    //Mesh::ptr mesh = mesh_asset->get_mesh();
    // set_local_transform(mesh_asset->local_transform);

    if (mesh_asset->primitive)
		set_primitive(mesh_asset->primitive->clone());
		
    update_nodes();
    //mesh_root(mesh_asset->root_node);
}





std::vector<RaytracingAccelerationStructure::ptr> MeshAssetInstance::create_raytracing_as()
{
	//std::vector<RaytracingAccelerationStructure::ptr> result;
	
	for (auto& info : rendering)
	{
	//	if (info.node_index == 3) continue;
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Triangles.IndexBuffer = universal_index_manager::get().buffer->get_gpu_address() + info.draw_arguments.StartIndexLocation * sizeof(UINT32);
		geometryDesc.Triangles.IndexCount = info.draw_arguments.IndexCountPerInstance;
		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometryDesc.Triangles.Transform3x4 = universal_nodes_manager::get().buffer->get_gpu_address() + info.mesh_info.GetNode_offset() * sizeof(Table::node_data::CB);
		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	//	geometryDesc.Triangles.VertexCount = info.
		geometryDesc.Triangles.VertexBuffer.StartAddress = universal_vertex_manager::get().buffer->get_gpu_address() + info.mesh_info.GetVertex_offset() * sizeof(Vertex);
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC > descs;
		descs.push_back(geometryDesc);


		RaytracingAccelerationStructure::ptr structure = std::make_shared<RaytracingAccelerationStructure>(descs);
		raytracing_as.push_back(structure);
		structure->srvs = DescriptorHeapManager::get().get_csu_static()->create_table(2);
	//	mesh_asset->vertex_buffer->place_structured_srv(structure->srvs[0], sizeof(Vertex), info.mesh->vertex_offset, info.mesh->vertex_count);
	//	mesh_asset->index_buffer->place_structured_srv(structure->srvs[1], sizeof(UINT), info.mesh->index_offset, info.mesh->index_count);
	//	structure->material = info.mesh->material;

		//break;
	}

	return raytracing_as;
}

BOOST_CLASS_EXPORT_IMPLEMENT(MeshAsset);
BOOST_CLASS_EXPORT_IMPLEMENT(AssetReference<MeshAsset>);

BOOST_CLASS_EXPORT_IMPLEMENT(MeshAssetInstance);




template void MeshAssetInstance::serialize(serialization_oarchive& arch, const unsigned int version);
template void MeshAssetInstance::serialize(serialization_iarchive& arch, const unsigned int version);

template void AssetReference<MeshAsset>::serialize(serialization_oarchive& arch, const unsigned int version);
template void AssetReference<MeshAsset>::serialize(serialization_iarchive& arch, const unsigned int version);

void SceneFrameManager::prepare(CommandList::ptr& command_list, Scene& scene)
{
	auto timer = command_list->start(L"Upload data");

	universal_nodes_manager::get().prepare(command_list);
	universal_vertex_manager::get().prepare(command_list);
	universal_index_manager::get().prepare(command_list);
	universal_material_manager::get().prepare(command_list);

//	universal_mesh_info_part_manager::get().prepare(command_list);
	  universal_material_info_part_manager::get().prepare(command_list);

	scene.mesh_infos->prepare(command_list);
}
