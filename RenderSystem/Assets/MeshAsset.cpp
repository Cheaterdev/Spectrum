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

		for (auto m : r->meshes)
		{
			auto p = meshes[m].primitive;
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
    overrided_material[i]->set(this->type,context);
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

MeshAssetInstance::~MeshAssetInstance()
{
}

bool MeshAssetInstance::update_transforms()
{
	bool res = scene_object::update_transforms();

	if (res)
	{

//		node_buffer.data().clear();
//	node_buffer.data().reserve(mesh_asset->nodes.size());
	//	nodes.clear();


	
	for (auto &e : rendering)
	{
		auto m = e.mesh;
		auto p = m->primitive;

		if (!e.primitive_global)
			e.primitive_global = std::make_shared<AABB>();

		mat4x4 mat = nodes[e.node_index].asset_node->mesh_matrix*global_transform;

		e.primitive_global->apply_transform(p, mat);
		node_buffer.data()[e.node_index] = mat;
		e.owner = this;

	}

	if (scene)
		scene->on_moved(this);
}
return res;
}


void MeshAssetInstance::update_nodes()
{
//	primitive.reset(new AABB());
	

	/////////////////

   node_buffer.data().clear();
    node_buffer.data().reserve(mesh_asset->nodes.size());
	nodes_indexes.resize(mesh_asset->nodes.size());
    nodes.clear();
    rendering.clear();
	int index = 0;
    std::function<bool(MeshNode*)> f = [this,&index](MeshNode * node)->bool
    {
		node->index = index++;// node_buffer.data().size();
        node_buffer.data().emplace_back(global_transform*node->mesh_matrix);
        nodes.emplace_back(node);

        for (auto& m : node->meshes)
            rendering.push_back({ &mesh_asset->meshes[m], node->index ,0,0,0,false, this});



        return true;
    };
    mesh_asset->root_node.iterate(f);
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
void MeshAssetInstance::draw(MeshRenderContext::ptr context)
{
    // std::lock_guard<std::mutex> g(m);
    context->set_nodes_buffer(node_buffer);
    //context.set
    /*  auto& graph = context->list->get_graphics();
      graph.set_index_buffer(mesh_asset->index_buffer->get_index_buffer_view(true));

      for (auto mesh : mesh_asset->meshes)
      {
          overrided_material[mesh.material]->set(context);
          context->draw_indexed(mesh.index_count, mesh.index_offset, mesh.vertex_offset);
      }*/
}





std::vector<RaytracingAccelerationStructure::ptr> MeshAssetInstance::create_raytracing_as(D3D12_GPU_VIRTUAL_ADDRESS address)
{
	//std::vector<RaytracingAccelerationStructure::ptr> result;

	for (auto& info : rendering)
	{
	//	if (info.node_index == 3) continue;
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Triangles.IndexBuffer = mesh_asset->index_buffer->get_gpu_address() +info.mesh->index_offset * sizeof(UINT32);
		geometryDesc.Triangles.IndexCount = info.mesh->index_count;
		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometryDesc.Triangles.Transform3x4 =  address + info.node_index * sizeof(gpu_cached_renderer::nodes);
		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.Triangles.VertexCount = info.mesh->vertex_count;
		geometryDesc.Triangles.VertexBuffer.StartAddress = mesh_asset->vertex_buffer->get_gpu_address() +info.mesh->vertex_offset * mesh_asset->vertex_buffer->get_stride();
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = mesh_asset->vertex_buffer->get_stride();
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC > descs;
		descs.push_back(geometryDesc);


		RaytracingAccelerationStructure::ptr structure = std::make_shared<RaytracingAccelerationStructure>(descs);
		raytracing_as.push_back(structure);
		structure->srvs = DescriptorHeapManager::get().get_csu_static()->create_table(2);
		mesh_asset->vertex_buffer->place_structured_srv(structure->srvs[0], sizeof(Vertex), info.mesh->vertex_offset, info.mesh->vertex_count);
		mesh_asset->index_buffer->place_structured_srv(structure->srvs[1], sizeof(UINT), info.mesh->index_offset, info.mesh->index_count);
		structure->material = info.mesh->material;

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
