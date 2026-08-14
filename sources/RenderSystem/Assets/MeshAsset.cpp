module Graphics:MeshAsset;
import RenderSystem;

import :Scene;
import :AssetRenderer;
import :Materials.UniversalMaterial;
import :RTX;

import HAL;
using namespace HAL;

// Engine mat4x4 is row-vector (v' = v*M, translation in row 3). DXR's transform
// buffers (D3D12_RAYTRACING_TRANSFORM3X4 / INSTANCE_DESC::Transform) are row-major
// 3x4 for column-vector convention (worldPos = M*objectPos) - transpose and drop
// the implicit last row (0,0,0,1) to convert.
static mat3x4 to_transform3x4(const mat4x4& m)
{
	mat3x4 result;

	for (int r = 0; r < 3; r++)
		for (int c = 0; c < 4; c++)
			result.rows[r][c] = m.rows[c][r];

	return result;
}

REGISTER_TYPE(MeshAsset);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, MeshAsset);
CEREAL_REGISTER_DYNAMIC_INIT(MeshAsset);
CEREAL_FORCE_REGISTER(MeshAsset);
CEREAL_FORCE_REGISTER_RELATION(Asset, MeshAsset);

REGISTER_TYPE(MeshAssetInstance);
CEREAL_REGISTER_POLYMORPHIC_RELATION(scene_object, MeshAssetInstance);
CEREAL_FORCE_REGISTER(MeshAssetInstance);
CEREAL_FORCE_REGISTER_RELATION(scene_object, MeshAssetInstance);
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
			ASSERT(p);
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
	FenceWaiter last;
	int i = 0;
	if (RenderSystem::get().device().is_rtx_supported())
		for (auto& mesh : meshes)
		{
			auto list = (RenderSystem::get().device().get_queue(CommandListType::DIRECT)->get_free_list());
			list->begin(L"RTX");
			auto mat = list->place_raw(to_transform3x4(nodes[mesh.node_index]->mesh_matrix));
			GeometryDesc geometryDesc = {};
			geometryDesc.Type = HAL::GeometryType::TRIANGLES;
			geometryDesc.IndexBuffer = mesh.index_buffer_view.get_resource_address();
			geometryDesc.IndexCount = static_cast<uint>(mesh.index_buffer_view.desc.size/sizeof(UINT));
			geometryDesc.IndexFormat = Format::R32_UINT;
			geometryDesc.Transform3x4 = mat;
			geometryDesc.VertexFormat = Format::R32G32B32_FLOAT;
			geometryDesc.VertexBuffer = mesh.vertex_buffer_view.get_resource_address();
			geometryDesc.VertexCount = static_cast<uint>(mesh.vertex_buffer_view.desc.size / sizeof(Table::mesh_vertex_input));
			geometryDesc.VertexStrideInBytes = sizeof(Table::mesh_vertex_input);
			geometryDesc.Flags = HAL::GeometryFlags::OPAQUE;
			std::vector<GeometryDesc > descs;
			descs.push_back(geometryDesc);
			mesh.ras = std::make_shared<RaytracingAccelerationStructure>(RenderSystem::get().device(), descs, list);
			last=list->execute();
			i++;
		}
	last.wait();
}
MeshAsset::MeshAsset(std::wstring file_name, AssetLoadingContext::ptr c)
{
	auto task = TaskInfoManager::get().create_task(file_name);
	if (!c) c = std::make_shared<AssetLoadingContext>();
	c->loading_task = task;
	auto data = MeshData::get_resource(convert(file_name), c);

	c->loading_task = nullptr;
	if (!data) return;


	//meshes = data->meshes;
	root_node = data->root_node;
	local_transform.identity();
	root_node.iterate([this](MeshNode* node)
		{
			nodes.push_back(node);

			return true;
		});



	for (auto& m : data->materials)
		materials.emplace_back(register_asset(m));

	primitive = data->primitive->clone();
	/*(AABB*)primitive.get())->min = { -5, -5, -5 };
	((AABB*)primitive.get())->max = { 5, 5, 5 };*/
	name = file_name.substr(file_name.find_last_of(L"\\") + 1);


	UINT unique_ids = 0;
	UINT primitive_ids = 0;

	//UINT meshlets_count = 0;

	std::vector<UINT32> unique_ids_buffer;
	std::vector<UINT32> priimitive_ids_buffer;


	std::vector<Table::Meshlet> meshlets;
	std::vector<Table::MeshletCullData> meshlet_cull;
	
	for (auto& mesh : data->meshes)
	{
	//	mesh.meshlets_offset = meshlets_count;
	//	meshlets_count += static_cast<UINT>(mesh.meshlets.size());


		for (auto& meshet : mesh.meshlets)
		{

			Table::Meshlet::Compiled meshlet_row;
			meshlet_row.primitiveCount = static_cast<UINT>(meshet.PrimitiveIndices.size() / 3);
			meshlet_row.primitiveOffset = primitive_ids / 3;

			meshlet_row.vertexCount = static_cast<UINT>(meshet.UniqueVertexIndices.size());
			meshlet_row.vertexOffset = unique_ids;

			meshlets.emplace_back(meshlet_row);

			meshlet_cull.emplace_back(meshet.cull_data);
			unique_ids_buffer.insert(unique_ids_buffer.end(), meshet.UniqueVertexIndices.begin(), meshet.UniqueVertexIndices.end());
			priimitive_ids_buffer.insert(priimitive_ids_buffer.end(), meshet.PrimitiveIndices.begin(), meshet.PrimitiveIndices.end());


			unique_ids += static_cast<UINT>(meshet.UniqueVertexIndices.size());
			primitive_ids += static_cast<UINT>(meshet.PrimitiveIndices.size());

		}

	}

	LinearAllocator allocator;

	auto vertex_handle = allocator.Allocate<Table::mesh_vertex_input>(data->vertex_buffer.size());
	auto index_handle = allocator.Allocate<uint>(data->index_buffer.size());
	auto unique_index_handle = allocator.Allocate<uint>(unique_ids_buffer.size());
	auto primitive_index_handle = allocator.Allocate<uint>(priimitive_ids_buffer.size());
	auto meshlet_handle = allocator.Allocate<Table::Meshlet>(meshlets.size());
	auto meshlet_cull_handle = allocator.Allocate<Table::MeshletCullData>(meshlet_cull.size());


	buffer = std::make_shared<HAL::Buffer>(RenderSystem::get().device(), HAL::ResourceDesc::Buffer(allocator.get_max_usage(), HAL::ResFlags::ShaderResource | HAL::ResFlags::Immutable), HAL::HeapType::DEFAULT);



	vertex_buffer_view = buffer->create_view<HAL::StructuredBufferView<Table::mesh_vertex_input>>(
		RenderSystem::get().device().get_static_gpu_data(),
		StructuredBufferViewDesc{
			vertex_handle.get_offset(),
			vertex_handle.get_size(),
			counterType::NONE
		});


	index_buffer_view = buffer->create_view<HAL::StructuredBufferView<UINT32>>(
		RenderSystem::get().device().get_static_gpu_data(),
		StructuredBufferViewDesc{
			index_handle.get_offset(),
			index_handle.get_size(),
			counterType::NONE
		});

	this->meshlets = buffer->create_view<HAL::StructuredBufferView<Table::Meshlet>>(
		RenderSystem::get().device().get_static_gpu_data(),
		StructuredBufferViewDesc{
			meshlet_handle.get_offset(),
			meshlet_handle.get_size(),
			counterType::NONE
		});
	meshlet_cull_datas = buffer->create_view<HAL::StructuredBufferView<Table::MeshletCullData>>(
		RenderSystem::get().device().get_static_gpu_data(),
		StructuredBufferViewDesc{
			meshlet_cull_handle.get_offset(),
			meshlet_cull_handle.get_size(),
			counterType::NONE
		});

	unique_indices = buffer->create_view<HAL::StructuredBufferView<UINT32>>(
		RenderSystem::get().device().get_static_gpu_data(),
		StructuredBufferViewDesc{
			unique_index_handle.get_offset(),
			unique_index_handle.get_size(),
			counterType::NONE
		});

	primitive_indices = buffer->create_view<HAL::StructuredBufferView<UINT32>>(
		RenderSystem::get().device().get_static_gpu_data(),
		StructuredBufferViewDesc{
			primitive_index_handle.get_offset(),
			primitive_index_handle.get_size(),
			counterType::NONE
		});


	auto list = (RenderSystem::get().device().get_upload_list());
	list->get_copy().update<Table::mesh_vertex_input>(vertex_buffer_view, 0, data->vertex_buffer);
	list->get_copy().update<UINT32>(index_buffer_view, 0, data->index_buffer);
	list->get_copy().update<Table::Meshlet>(this->meshlets, 0, meshlets);
	list->get_copy().update<Table::MeshletCullData>(meshlet_cull_datas, 0, meshlet_cull);
	list->get_copy().update<UINT32>(unique_indices, 0, unique_ids_buffer);
	list->get_copy().update<UINT32>(primitive_indices, 0, priimitive_ids_buffer);

	list->execute_and_wait();

	meshes.reserve(data->meshes.size());

	uint64 meshlets_offset= 0;
	for (auto& mesh : data->meshes)
	{
		meshes.emplace_back();
		auto & compiled=meshes.back();
		compiled.primitive = mesh.primitive->clone();
	compiled.material = mesh.material;

			compiled.vertex_buffer_view = buffer->create_view<HAL::StructuredBufferView<Table::mesh_vertex_input>>(
				RenderSystem::get().device().get_static_gpu_data(),
				StructuredBufferViewDesc{
					vertex_buffer_view.desc.offset + mesh.vertex_offset * sizeof(Table::mesh_vertex_input),
					mesh.vertex_count * sizeof(Table::mesh_vertex_input),
					counterType::NONE
				});


			compiled.index_buffer_view = buffer->create_view<HAL::StructuredBufferView<UINT32>>(
				RenderSystem::get().device().get_static_gpu_data(),
				StructuredBufferViewDesc{
					index_buffer_view.desc.offset  + mesh.index_offset * sizeof(UINT32),
					mesh.index_count * sizeof(UINT32),
					counterType::NONE
				});
			compiled.meshet_view = buffer->create_view<HAL::StructuredBufferView<Table::Meshlet>>(
				RenderSystem::get().device().get_static_gpu_data(),
				StructuredBufferViewDesc{
					this->meshlets.desc.offset  + meshlets_offset* sizeof(Table::Meshlet),
					mesh.meshlets.size() * sizeof(Table::Meshlet),
					counterType::NONE
				});

			compiled.draw_arguments.StartIndexLocation = static_cast<uint>(compiled.index_buffer_view.desc.offset/sizeof(UINT));
			compiled.draw_arguments.IndexCountPerInstance = mesh.index_count;
			compiled.draw_arguments.BaseVertexLocation = 0;
			compiled.draw_arguments.InstanceCount = 1;
			compiled.draw_arguments.StartInstanceLocation = 0;


			compiled.dispatch_mesh_arguments.ThreadGroupCountX = static_cast<UINT>(Math::DivideByMultiple(mesh.meshlets.size(), 32));
			compiled.dispatch_mesh_arguments.ThreadGroupCountY = 1;
			compiled.dispatch_mesh_arguments.ThreadGroupCountZ = 1;

		meshlets_offset+=mesh.meshlets.size() ;
	}
		for (int i = 0; i < nodes.size(); i++)
	{
		auto m = nodes[i]->mesh_id;
		if (m != -1)
			meshes[m].node_index = i;
	}

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


void MeshAsset::update_preview(HAL::Texture::ptr preview)
{
	if (!preview || !preview->is_rt())
		preview.reset(new HAL::Texture(RenderSystem::get().device(), HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { 256, 256 }, 1, 6, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess)));

	if (!preview_mesh)
		preview_mesh.reset(new MeshAssetInstance(get_ptr<MeshAsset>()));

	AssetRenderer::get().draw(preview_mesh, preview);
	mark_changed();
}

template<class Archive>
void MeshAssetInstance::serialize(Archive& ar)
{
	SAVE_PARENT(AssetHolder);
	SAVE_PARENT(scene_object);

	ar& NVP(overrided_material);
	ar& NVP(mesh_asset);
	ar& NVP(type);

	IF_LOAD()
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
	meshpart[0].mesh_cb = info.compiled_mesh_info.compiled();
	meshpart[0].meshinstance_cb = mesh_instance_info.compiled();

	meshpart[0].draw_commands = info.dispatch_mesh_arguments;
	meshpart[0].node_offset = info.mesh_info.GetNode_offset();

	meshpart_handle.write(i, meshpart);

	// The raster path reads material_id out of meshpart (just written
	// above) fresh every frame, but RTX picks its shader via the TLAS
	// instance's hit_group_index, which is only baked in update_rtx_instance()
	// -- called from on_add() and from update_transforms() (and only there
	// if the object actually moved). Without this, a material override only
	// reaches RTX rays if the object happens to move afterward; it otherwise
	// keeps hitting the old material's shader record.
	if (scene)
	{
		update_rtx_instance();
		scene->on_changed(this);
	}
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

	// Compute real world bounds/transform before scene_object::on_add()
	// below fires on_element_add -- otherwise listeners (VSM's invalidation
	// tracker) observe this object's still-default (0,0,0) bounds instead of
	// its real placement, since update_transforms() normally only runs on
	// the next per-frame scene update pass (Scene::update_transforms(),
	// called once/frame from think()), not at insertion time.
	// this->parent/this->scene are set here so update_transforms() has what
	// it needs (parent->global_transform, and MeshAssetInstance's own
	// override calls update_rtx_instance() which needs a valid scene) --
	// harmlessly redundant with parent_type::on_add() doing the same
	// assignment again inside scene_object::on_add() right below.
	if (!old_scene && parent && parent->scene)
	{
		this->parent = parent;
		this->scene  = parent->scene;
		update_transforms();
	}

	scene_object::on_add(parent);

	if (!old_scene && scene)
	{

		Scene* render_scene = dynamic_cast<Scene*>(scene);

		render_scene->mesh_infos->allocate(meshpart_handle, rendering_count);
		auto meshpart = meshpart_handle.map();//  render_scene->mesh_infos->map_elements(meshpart_handle.get_offset(), rendering_count);



		int i = 0;
		for (auto& info : rendering)
		{

			meshpart[i].mesh_cb = info.compiled_mesh_info.compiled();
			meshpart[i].meshinstance_cb = mesh_instance_info.compiled();

			meshpart[i].material_id = static_cast<materials::universal_material*>(info.material)->get_material_id();
			meshpart[i].draw_commands = info.dispatch_mesh_arguments;
			meshpart[i].node_offset = info.mesh_info.GetNode_offset();


			render_scene->command_ids[static_cast<int>(type)].insert(static_cast<UINT>(meshpart_handle.get_offset()) + i);
			render_scene->command_ids[static_cast<int>(MESH_TYPE::ALL)].insert(static_cast<UINT>(meshpart_handle.get_offset()) + i);


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
			old_scene->command_ids[static_cast<int>(type)].erase(static_cast<UINT>(meshpart_handle.get_offset()) + i);
			old_scene->command_ids[static_cast<int>(MESH_TYPE::ALL)].erase(static_cast<UINT>(meshpart_handle.get_offset()) + i);
			i++;
		}
		meshpart_handle.Free();
		meshpart_handle = TypedHandle<Table::MeshCommandData>();
	}
}

void MeshAssetInstance::update_rtx_instance()
{
	if (!RenderSystem::get().device().is_rtx_supported()) return;

	if (!ras_handle) scene->raytrace->allocate(ras_handle, rendering_count);

	if (ras_handle)
	{
		auto ras = ras_handle.map();

		int i = 0;
		for (auto& info : rendering)
		{


			HAL::InstanceDesc instanceDesc = {};

			instanceDesc.transform = to_transform3x4(global_transform);

			instanceDesc.mask = 1;
			instanceDesc.acceleration_structure = info.ras->get_gpu_address().get_ptr();
			instanceDesc.instance_id = info.node_id;
			instanceDesc.hit_group_index = RTX::get().rtx.get_index(static_cast<materials::universal_material*>(info.material));


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
			/*	uint index = static_cast<UINT>(info.mesh_info.GetNode_offset());*/
			uint index = static_cast<UINT>(info.mesh_info.GetNode_offset()) - static_cast<UINT>(nodes_handle.get_offset());
			auto& p = info.primitive;

			if (!info.primitive_global)
				info.primitive_global = std::make_shared<AABB>();

			mat4x4 prev_mat = info.global_mat;
			info.global_mat = nodes[index].asset_node->mesh_matrix * global_transform;
			info.primitive_global->apply_transform(p, info.global_mat);

			//auto& my_node = gpu_nodes[info.mesh_info.GetNode_offset()];
			auto& my_node = gpu_nodes[static_cast<UINT>(info.mesh_info.GetNode_offset() - nodes_handle.get_offset())];
			my_node.node_global_matrix = info.global_mat;
			my_node.node_global_matrix_prev = prev_mat;

			my_node.node_inverse_matrix = info.global_mat;
			my_node.node_inverse_matrix.inverse();


			my_node.aabb.min = float4(info.primitive->get_min(), 0);
			my_node.aabb.max = float4(info.primitive->get_max(), 0);


			need_update_mats = prev_mat != info.global_mat;

		}


		update_rtx_instance();

		nodes_handle.write(0, gpu_nodes);
		if (scene)
			scene->on_moved(this);


	}
	return res;
}


bool MeshAssetInstance::init_ras(HAL::CommandList::ptr list)
{
	if (!RenderSystem::get().device().is_rtx_supported()) return false;



	return true;
}

void MeshAssetInstance::update_nodes()
{
	//	primitive.reset(new AABB());

	nodes_count = 0;
	rendering_count = 0;

	mesh_asset->root_node.iterate([&](MeshNode* m) {nodes_count++; rendering_count += (m->mesh_id != -1); return true; });


	universal_nodes_manager::get().allocate(nodes_handle, nodes_count);
	universal_rtx_manager::get().allocate(instance_handle, rendering_count);


	nodes_buffer_view = universal_nodes_manager::get().buffer.resource->create_view<HAL::StructuredBufferView<Table::node_data>>(
		RenderSystem::get().device().get_static_gpu_data(),
		StructuredBufferViewDesc{
			(uint)nodes_handle.get_offset_in_bytes(),
			(uint)nodes_handle.get_size_in_bytes(),
			counterType::NONE
		});
	Slots::MeshInstanceInfo instance_info;

	instance_info.GetVertexes() = mesh_asset->vertex_buffer_view;
	instance_info.GetIndices() = mesh_asset->index_buffer_view;
	instance_info.GetMeshletCullData() = mesh_asset->meshlet_cull_datas;
	instance_info.GetMeshlets() = mesh_asset->meshlets;

	instance_info.GetUnique_indices() = mesh_asset->unique_indices;
	instance_info.GetPrimitive_indices() = mesh_asset->primitive_indices;

	mesh_instance_info = instance_info.compile(RenderSystem::get().device().get_static_gpu_data());
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

			info.vertex_buffer_view = mesh_asset->meshes[m].vertex_buffer_view;
			info.index_buffer_view = mesh_asset->meshes[m].index_buffer_view;
			info.draw_arguments = mesh_asset->meshes[m].draw_arguments;
			info.dispatch_mesh_arguments = mesh_asset->meshes[m].dispatch_mesh_arguments;


			info.primitive = mesh_asset->meshes[m].primitive;

			info.primitive_global = std::make_shared<AABB>();



			info.global_mat = nodes.back().asset_node->mesh_matrix * global_transform;

			info.primitive_global->apply_transform(info.primitive, info.global_mat);


			//	node_buffer[node->index] = mat;
				//if(nodes_ptr) 

			info.mesh_info.GetVertex_offset_local() = static_cast<UINT>(info.vertex_buffer_view.desc.offset/sizeof(Table::mesh_vertex_input));

			info.mesh_info.GetMeshlet_offset_local() = static_cast<UINT>((mesh_asset->meshes[m].meshet_view.desc.offset-mesh_asset->meshlets.desc.offset)/sizeof(Table::Meshlet));





			info.mesh_info.GetNode_offset() = static_cast<UINT>(nodes_handle.get_offset() + nodes.size() - 1);

			info.mesh_info.GetMeshlet_count() = static_cast<UINT>(mesh_asset->meshes[m].meshet_view.desc.size/sizeof(Table::Meshlet));

			info.meshlet_offset = info.mesh_info.GetMeshlet_offset_local();
			info.meshlet_count = info.mesh_info.GetMeshlet_count();

			/*	auto& my_node = gpu_nodes[static_cast<UINT>(info.mesh_info.GetNode_offset())];*/
			auto& my_node = gpu_nodes[static_cast<UINT>(info.mesh_info.GetNode_offset() - nodes_handle.get_offset())];
			my_node.node_global_matrix = info.global_mat;
			my_node.node_inverse_matrix = info.global_mat;
			my_node.node_inverse_matrix.inverse();

			my_node.node_global_matrix_prev = info.global_mat;

			my_node.aabb.min = float4(info.primitive->get_min(), 0);
			my_node.aabb.max = float4(info.primitive->get_max(), 0);

			info.material_id = static_cast<UINT>(mesh_asset->meshes[m].material);
			info.material = overrided_material[info.material_id]->get_ptr<MaterialAsset>().get();
			info.compiled_mesh_info = info.mesh_info.compile(RenderSystem::get().device().get_static_gpu_data());
			info.mesh_instance_info = mesh_instance_info;
			info.ras = mesh_asset->meshes[m].ras;

			//ASSERT(info.ras);

			info.node_id = static_cast<UINT>(instance_handle.get_offset() + nodes.size() - 1);
			auto& my_instance = gpu_instances[static_cast<UINT>(nodes.size()) - 1];
			my_instance.vertexes = info.vertex_buffer_view.structuredBuffer.get_offset();
			my_instance.indices = info.index_buffer_view.structuredBuffer.get_offset();

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


void MeshNode::iterate(std::function<bool(MeshNode*)> f)
{
	if (f(this))
	{
		for (auto& c : childs)
			c.iterate(f);
	}
}

void MeshAsset::shutdown()
{
	preview_mesh.reset();
}

MeshAsset::~MeshAsset()
{
	materials.clear();
}

void MeshAsset::reload_resource()
{
}

MeshAssetInstance::mesh_asset_node::mesh_asset_node(MeshNode* node)
{
	asset_node = node;
}

universal_rtx_manager::universal_rtx_manager() : HAL::virtual_gpu_buffer<Table::RaytraceInstanceInfo>(RenderSystem::get().device(), MAX_NODES_SIZE)
{
}

universal_mesh_instance_manager::universal_mesh_instance_manager() : HAL::virtual_gpu_buffer<Table::MeshInstanceInfo>(RenderSystem::get().device(), MAX_NODES_SIZE)
{
}

universal_nodes_manager::universal_nodes_manager() : HAL::virtual_gpu_buffer<Table::node_data>(RenderSystem::get().device(), MAX_NODES_SIZE)
{
}

universal_material_info_part_manager::universal_material_info_part_manager() : HAL::virtual_gpu_buffer<Table::MaterialCommandData>(RenderSystem::get().device(), MAX_COMMANDS_SIZE)
{
}

void SceneFrameManager::prepare(HAL::CommandList::ptr& command_list, Scene& scene)
{
	auto timer = command_list->start(L"Upload data");

	universal_nodes_manager::get().prepare(command_list);

	universal_mesh_instance_manager::get().prepare(command_list);
	//	universal_mesh_info_part_manager::get().prepare(command_list);
	universal_material_info_part_manager::get().prepare(command_list);
	universal_rtx_manager::get().prepare(command_list);

	scene.mesh_infos->prepare(command_list);
	scene.raytrace->prepare(command_list);

}
