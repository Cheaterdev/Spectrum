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
	std::function<bool(MeshNode*)> f = [&first, this](MeshNode* r)->bool
	{

		//for (auto m : r->meshes)

		if (r->mesh_id != -1)
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

//////////////////////////////////////////////////////////////////////////
Asset_Type MeshAsset::get_type()
{
	return Asset_Type::MESH;
}


void MeshAsset::init_gpu()
{

	
	{
		universal_vertex_manager::get().allocate(vertex_handle, vertex_buffer.size());
			vertex_handle.write(0, vertex_buffer);
	}

	{
		universal_index_manager::get().allocate(index_handle, index_buffer.size());
			index_handle.write(0, index_buffer);
	}


//	if (GetAsyncKeyState('8')) return;

	
	for (auto& mesh : meshes)
	{
		auto list = Device::get().get_queue(CommandListType::DIRECT)->get_free_list();
		list->begin("RTX");

		universal_vertex_manager::get().prepare(list);
		universal_index_manager::get().prepare(list);

		auto mat = list->place_raw(nodes[mesh.node_index]->mesh_matrix);


		GeometryDesc geometryDesc = {};
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

		geometryDesc.IndexBuffer = universal_index_manager::get().buffer->get_resource_address().offset(UINT(index_handle.get_offset() + mesh.index_offset) * sizeof(UINT32));
		geometryDesc.IndexCount = mesh.index_count;
		geometryDesc.IndexFormat = DXGI_FORMAT_R32_UINT;
		geometryDesc.Transform3x4 = mat.get_resource_address();//universal_nodes_manager::get().buffer->get_resource_address().offset(info.mesh_info.GetNode_offset() * sizeof(Table::node_data::CB));
		geometryDesc.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.VertexBuffer = universal_vertex_manager::get().buffer->get_resource_address().offset((vertex_handle.get_offset() + mesh.vertex_offset) * sizeof(Table::mesh_vertex_input::CB));
		geometryDesc.VertexStrideInBytes = sizeof(Table::mesh_vertex_input::CB);
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		std::vector<GeometryDesc > descs;
		descs.push_back(geometryDesc);

		mesh.ras = std::make_shared<RaytracingAccelerationStructure>(descs, list);
		list->execute_and_wait();
	}

	
}
MeshAsset::MeshAsset(std::wstring file_name, AssetLoadingContext::ptr c)
{
	auto task = TaskInfoManager::get().create_task(file_name);
	if (!c) c = std::make_shared<AssetLoadingContext>();
	c->loading_task = task;
	auto data = MeshData::get_resource(convert(file_name), c);

	c->loading_task = nullptr;
	if (!data) return;

	vertex_buffer = data->vertex_buffer;
	index_buffer = data->index_buffer;

	

	meshes = data->meshes;
	root_node = data->root_node;
	local_transform.identity();
	root_node.iterate([this](MeshNode* node)
		{
			nodes.push_back(node);
			
			return true;
		});


	for(int i = 0;i<nodes.size();i++)
	{
		auto m = nodes[i]->mesh_id;
		if(m != -1)
		meshes[m].node_index = i;
	}
	for (auto& m : data->materials)
		materials.emplace_back(register_asset(m));

	primitive = data->primitive->clone();
	/*(AABB*)primitive.get())->min = { -5, -5, -5 };
	((AABB*)primitive.get())->max = { 5, 5, 5 };*/
	name = file_name.substr(file_name.find_last_of(L"\\") + 1);

	init_gpu();
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
	mark_changed();
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

MeshAssetInstance::MeshAssetInstance() : mesh_asset(this)
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
	meshpart[0].node_offset = info.mesh_info.GetNode_offset();

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

	if (!old_scene && scene)
	{

		Scene* render_scene = dynamic_cast<Scene*>(scene);

		render_scene->mesh_infos->allocate(meshpart_handle, rendering_count);
		auto meshpart = meshpart_handle.map();//  render_scene->mesh_infos->map_elements(meshpart_handle.get_offset(), rendering_count);



		int i = 0;
		for (auto& info : rendering)
		{

			meshpart[i].mesh_cb = info.compiled_mesh_info.cb;
			meshpart[i].material_id = static_cast<materials::universal_material*>(info.material)->get_material_id();
			meshpart[i].draw_commands = info.draw_arguments;
			meshpart[i].node_offset = info.mesh_info.GetNode_offset();


			render_scene->command_ids[int(type)].insert((UINT)meshpart_handle.get_offset() + i);
			render_scene->command_ids[int(MESH_TYPE::ALL)].insert((UINT)meshpart_handle.get_offset() + i);


			i++;
		}

		meshpart_handle.write(0, meshpart);

		update_rtx_instance();
	}
}


void MeshAssetInstance::on_remove()
{
	auto old_scene = scene;
	scene_object::on_remove();

	if (old_scene && !scene)
	{
		int i = 0;
		for (auto& info : rendering)
		{
			old_scene->command_ids[int(type)].erase((UINT)meshpart_handle.get_offset() + i);
			old_scene->command_ids[int(MESH_TYPE::ALL)].erase((UINT)meshpart_handle.get_offset() + i);
			i++;
		}
		meshpart_handle.Free();
		meshpart_handle = TypedHandle<mesh_info_part::CB>();
	}
}

void MeshAssetInstance::update_rtx_instance()
{
	if (!Device::get().is_rtx_supported()) return;


	if (!ras_handle) scene->raytrace->allocate(ras_handle, rendering_count);

	if (ras_handle)
	{
		auto ras = ras_handle.map();

		int i = 0;
		for (auto& info : rendering)
		{


			D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};

			for (int x = 0; x < 3; x++)
				for (int y = 0; y < 4; y++)
				{
					instanceDesc.Transform[x][y] = global_transform.rows[y][x];
				}

			instanceDesc.InstanceMask = 1;
			instanceDesc.AccelerationStructure = info.ras->get_gpu_address();
			instanceDesc.InstanceID = info.node_id;
			instanceDesc.InstanceContributionToHitGroupIndex = static_cast<materials::universal_material*>(info.material)->info_rtx.get_offset();

			ras[i] = instanceDesc;

			i++;


		}
		ras_handle.write(0, ras);
	}
}

bool MeshAssetInstance::update_transforms()
{
	bool res = scene_object::update_transforms();

	if (res || need_update_mats)
	{

		auto gpu_nodes = nodes_handle.map();
		int i = 0;
		for (auto& info : rendering)
		{
			uint index = (UINT)info.mesh_info.GetNode_offset() - (UINT)nodes_handle.get_offset();
			auto& p = info.primitive;

			if (!info.primitive_global)
				info.primitive_global = std::make_shared<AABB>();

			mat4x4 prev_mat = info.global_mat;
			info.global_mat = nodes[index].asset_node->mesh_matrix * global_transform;
			info.primitive_global->apply_transform(p, info.global_mat);

			auto& my_node = gpu_nodes[info.mesh_info.GetNode_offset() - nodes_handle.get_offset()];
			my_node.node_global_matrix = info.global_mat;
			my_node.node_global_matrix_prev = prev_mat;

			my_node.node_inverse_matrix = info.global_mat;
			my_node.node_inverse_matrix.inverse();


			my_node.aabb.min = info.primitive->get_min();
			my_node.aabb.max = info.primitive->get_max();


			need_update_mats = prev_mat != info.global_mat;

		}


		update_rtx_instance();

		nodes_handle.write(0, gpu_nodes);
		if (scene)
			scene->on_moved(this);


	}
	return res;
}


bool MeshAssetInstance::init_ras(CommandList::ptr list)
{
	if (!Device::get().is_rtx_supported()) return false;



	return true;
}

void MeshAssetInstance::update_nodes()
{
	//	primitive.reset(new AABB());

	nodes_count = 0;
	rendering_count = 0;

	mesh_asset->root_node.iterate([&](MeshNode* m) {nodes_count++; rendering_count += (m->mesh_id != -1); return true; });


	universal_nodes_manager::get().allocate(nodes_handle, nodes_count);
	universal_mesh_instance_manager::get().allocate(instance_handle, rendering_count);

	auto gpu_nodes = nodes_handle.map();
	auto gpu_instances = instance_handle.map();

	nodes_indexes.resize(mesh_asset->nodes.size());
	nodes.clear();
	rendering.clear();


	std::function<bool(MeshNode*)> f = [&](MeshNode* node)->bool
	{

		if (node->mesh_id == -1) return true;
		int m = node->mesh_id;
		//	for (auto& m : node->meshes)
		{
			nodes.emplace_back(node);



			render_info info;

			info.draw_arguments.StartIndexLocation = UINT(mesh_asset->index_handle.get_offset() + mesh_asset->meshes[m].index_offset);
			info.draw_arguments.IndexCountPerInstance = mesh_asset->meshes[m].index_count;
			info.draw_arguments.BaseVertexLocation = 0;
			info.draw_arguments.InstanceCount = 1;
			info.draw_arguments.StartInstanceLocation = 0;

			info.primitive = mesh_asset->meshes[m].primitive;

			info.primitive_global = std::make_shared<AABB>();



			info.global_mat = nodes.back().asset_node->mesh_matrix * global_transform;

			info.primitive_global->apply_transform(info.primitive, info.global_mat);


			//	node_buffer[node->index] = mat;
				//if(nodes_ptr) 

			info.mesh_info.GetNode_offset() = UINT(nodes_handle.get_offset() + nodes.size() - 1);
			info.mesh_info.GetVertex_offset() = UINT(mesh_asset->vertex_handle.get_offset() + mesh_asset->meshes[m].vertex_offset);

			auto& my_node = gpu_nodes[UINT(info.mesh_info.GetNode_offset() - nodes_handle.get_offset())];
			my_node.node_global_matrix = info.global_mat;
			my_node.node_inverse_matrix = info.global_mat;
			my_node.node_inverse_matrix.inverse();

			my_node.node_global_matrix_prev = info.global_mat;

			my_node.aabb.min = info.primitive->get_min();
			my_node.aabb.max = info.primitive->get_max();

			info.material_id = UINT(mesh_asset->meshes[m].material);
			info.material = overrided_material[info.material_id]->get_ptr<MaterialAsset>().get();
			info.compiled_mesh_info = info.mesh_info.compile(StaticCompiledGPUData::get());

			info.ras = mesh_asset->meshes[m].ras;

			assert(info.ras);

			info.node_id = (UINT)(instance_handle.get_offset() + nodes.size() - 1);
			auto& my_instance = gpu_instances[(UINT)nodes.size() - 1];
			my_instance.index_offset = info.draw_arguments.StartIndexLocation;
			my_instance.vertex_offset = info.mesh_info.GetVertex_offset();

			rendering.push_back(info);
		}


		return true;
	};

	mesh_asset->root_node.iterate(f);

	nodes_handle.write(0, gpu_nodes);

	instance_handle.write(0, gpu_instances);

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

	universal_mesh_instance_manager::get().prepare(command_list);
	//	universal_mesh_info_part_manager::get().prepare(command_list);
	universal_material_info_part_manager::get().prepare(command_list);

	scene.mesh_infos->prepare(command_list);
	scene.raytrace->prepare(command_list);

}
