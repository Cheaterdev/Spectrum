module Graphics:Scene;
import RenderSystem;

import :Materials.UniversalMaterial;
import :MeshAsset;

import HAL;
using namespace HAL;

void Scene::iterate_meshes(MESH_TYPE mesh_type, std::function<void(scene_object::ptr)> f)
{
    if (mesh_type & MESH_TYPE::STATIC)
        for (auto& instance : static_objects)
            f(instance->get_ptr<scene_object>());

    if (mesh_type & MESH_TYPE::DYNAMIC)
        for (auto& instance : dynamic_objects)
            f(instance->get_ptr<scene_object>());
}

Scene::~Scene()
{
	remove_all();
}

Scene::Scene()
{
	scene = this;
	// controller = this;
	on_element_add.register_handler(this, [this](scene_object* object) {

		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			static_objects.insert(render_object);

		if (render_object->type == MESH_TYPE::DYNAMIC)
			dynamic_objects.insert(render_object);

		invalidate_scene_caches();
		});

	on_element_remove.register_handler(this, [this](scene_object* object) {

		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			static_objects.erase(render_object);

		if (render_object->type == MESH_TYPE::DYNAMIC)
			dynamic_objects.erase(render_object);

		invalidate_scene_caches();
		});

	on_moved.register_handler(this, [this](scene_object* object) {

		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			bvh_static_dirty = true;
		else
			bvh_dynamic_dirty = true;
		});

	mesh_infos = std::make_shared< virtual_gpu_buffer<Table::Meshes::MeshCommandData>>(RenderSystem::get().device(), 1024 * 1024);
	raytrace = std::make_shared<virtual_gpu_buffer<HAL::InstanceDesc>>(RenderSystem::get().device(), 1024 * 1024);


	if (RenderSystem::get().device().is_ray_query_supported())
	{
		std::vector<InstanceDesc>  desc;
		raytrace_scene = std::make_shared<RaytracingAccelerationStructure>(RenderSystem::get().device(), desc);
	}
}

void Scene::invalidate_scene_caches()
{
	scene_caches_dirty = true;
	bvh_static_dirty = true;
	bvh_dynamic_dirty = true;
}

namespace
{
	struct BvhItem
	{
		float lo[3];
		float hi[3];
		uint id;

		float center(int axis) const { return (lo[axis] + hi[axis]) * 0.5f; }
	};

	// Nodes of the tree over items[begin, end), depth first: the left child follows its parent. Returns the
	// node's index. Splits at the median of the longest axis of the centers, so the depth is log2 of the count.
	uint32_t emit_bvh_node(std::vector<BvhItem>& items, size_t begin, size_t end, std::vector<uint32_t>& nodes)
	{
		float lo[3] = { items[begin].lo[0], items[begin].lo[1], items[begin].lo[2] };
		float hi[3] = { items[begin].hi[0], items[begin].hi[1], items[begin].hi[2] };
		float center_lo[3] = { items[begin].center(0), items[begin].center(1), items[begin].center(2) };
		float center_hi[3] = { center_lo[0], center_lo[1], center_lo[2] };
		for (size_t i = begin + 1; i < end; i++)
			for (int a = 0; a < 3; a++)
			{
				lo[a] = std::min(lo[a], items[i].lo[a]);
				hi[a] = std::max(hi[a], items[i].hi[a]);
				center_lo[a] = std::min(center_lo[a], items[i].center(a));
				center_hi[a] = std::max(center_hi[a], items[i].center(a));
			}

		const uint32_t index = static_cast<uint32_t>(nodes.size() / 8);
		nodes.resize(nodes.size() + 8);
		auto write = [&](uint32_t a, uint32_t b)
		{
			uint32_t* n = &nodes[index * 8ull];
			for (int k = 0; k < 3; k++)
			{
				n[k] = std::bit_cast<uint32_t>(lo[k]);
				n[4 + k] = std::bit_cast<uint32_t>(hi[k]);
			}
			n[3] = a;
			n[7] = b;
		};

		if (end - begin == 1)
		{
			write(items[begin].id, 0);
			return index;
		}

		int axis = 0;
		for (int a = 1; a < 3; a++)
			if (center_hi[a] - center_lo[a] > center_hi[axis] - center_lo[axis])
				axis = a;

		const size_t mid = begin + (end - begin) / 2;
		std::nth_element(items.begin() + begin, items.begin() + mid, items.begin() + end,
			[axis](const BvhItem& l, const BvhItem& r) { return l.center(axis) < r.center(axis); });

		const uint32_t left = emit_bvh_node(items, begin, mid, nodes);
		const uint32_t right = emit_bvh_node(items, mid, end, nodes);
		write(left, right);
		return index;
	}

	// Splits the same way until a range fits a treelet; the splits above the treelets are not stored.
	void emit_bvh_treelets(std::vector<BvhItem>& items, size_t begin, size_t end, std::vector<uint32_t>& nodes, std::vector<uint32_t>& treelets)
	{
		if (end <= begin)
			return;

		if (end - begin <= Scene::Bvh::TreeletLeaves)
		{
			treelets.push_back(emit_bvh_node(items, begin, end, nodes));
			return;
		}

		int axis = 0;
		float extent = -1;
		for (int a = 0; a < 3; a++)
		{
			float lo = items[begin].center(a), hi = lo;
			for (size_t i = begin + 1; i < end; i++)
			{
				lo = std::min(lo, items[i].center(a));
				hi = std::max(hi, items[i].center(a));
			}
			if (hi - lo > extent)
			{
				extent = hi - lo;
				axis = a;
			}
		}

		const size_t mid = begin + (end - begin) / 2;
		std::nth_element(items.begin() + begin, items.begin() + mid, items.begin() + end,
			[axis](const BvhItem& l, const BvhItem& r) { return l.center(axis) < r.center(axis); });

		emit_bvh_treelets(items, begin, mid, nodes, treelets);
		emit_bvh_treelets(items, mid, end, nodes, treelets);
	}
}

void Scene::update_bvh()
{
	std::lock_guard lock(bvh_mutex);

	if (!bvh_static_dirty && !bvh_dynamic_dirty)
		return;

	PROFILE(L"scene_update_bvh");

	if (scene_caches_dirty)
		rebuild_scene_caches();

	auto rebuild = [&](MESH_TYPE type, BvhPart& part)
	{
		std::vector<BvhItem> items;
		for (auto object : mesh_objects)
		{
			if (object->type != type)
				continue;

			uint index = 0;
			for (auto& info : object->rendering)
			{
				BvhItem item;
				item.id = object->first_mesh_id() + index++;
				auto lo = info.primitive_global->get_min();
				auto hi = info.primitive_global->get_max();
				item.lo[0] = lo.x; item.lo[1] = lo.y; item.lo[2] = lo.z;
				item.hi[0] = hi.x; item.hi[1] = hi.y; item.hi[2] = hi.z;
				items.push_back(item);
			}
		}

		part.nodes.clear();
		part.treelets.clear();
		part.nodes.reserve(items.size() * 16);
		emit_bvh_treelets(items, 0, items.size(), part.nodes, part.treelets);
	};

	if (bvh_static_dirty)
		rebuild(MESH_TYPE::STATIC, bvh_static);
	if (bvh_dynamic_dirty)
		rebuild(MESH_TYPE::DYNAMIC, bvh_dynamic);
	bvh_static_dirty = false;
	bvh_dynamic_dirty = false;

	// The dynamic part's node indices are relative to it; they follow the static part's.
	bvh.nodes = bvh_static.nodes;
	bvh.treelets = bvh_static.treelets;
	bvh.static_treelets = static_cast<uint>(bvh_static.treelets.size());
	const uint32_t offset = static_cast<uint32_t>(bvh_static.nodes.size() / 8);
	const size_t first_dynamic = bvh.nodes.size();
	bvh.nodes.insert(bvh.nodes.end(), bvh_dynamic.nodes.begin(), bvh_dynamic.nodes.end());
	for (size_t n = first_dynamic; n < bvh.nodes.size(); n += 8)
		if (bvh.nodes[n + 7] != 0)
		{
			bvh.nodes[n + 3] += offset;
			bvh.nodes[n + 7] += offset;
		}
	for (auto root : bvh_dynamic.treelets)
		bvh.treelets.push_back(root + offset);

	bvh.version++;
}

namespace
{
	// Capacity of a SceneBvhCuller's GPU buffers: nodes are 8 dwords, at most 2 per mesh part.
	constexpr size_t BvhNodeDwords = 8 * 1024 * 1024;
	constexpr size_t BvhTreeletCapacity = 1024 * 1024;
	constexpr UINT BvhMaxParts = 1024 * 1024;
}

SceneBvhCuller::SceneBvhCuller()
{
	auto& device = RenderSystem::get().device();
	nodes    = std::make_shared<virtual_gpu_buffer<UINT>>(device, BvhNodeDwords, counterType::NONE, HAL::ResFlags::ShaderResource);
	treelets = std::make_shared<virtual_gpu_buffer<UINT>>(device, BvhTreeletCapacity, counterType::NONE, HAL::ResFlags::ShaderResource);
	ids      = std::make_shared<virtual_gpu_buffer<UINT>>(device, BvhMaxParts, counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
	args     = HAL::StructuredBufferView<DispatchArguments>(device, 1, counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);

	Slots::Meshes::GatherPipelineGlobal global;
	global.GetCommands() = ids->buffer.structuredBuffer;
	global.GetMeshes_count() = ids->buffer.counter_view;
	gather = global.compile(device.get_static_gpu_data());
}

bool SceneBvhCuller::cull(HAL::CommandList& list, Scene& scene, MESH_TYPE type, const Table::Frame::Frustum& volume)
{
	scene.update_bvh();
	auto& bvh = scene.bvh;

	// The buffers are fixed in size; a scene beyond them scans the whole list.
	if (bvh.treelets.empty() || bvh.nodes.size() > BvhNodeDwords || bvh.treelets.size() > BvhTreeletCapacity)
		return false;

	uint first = 0;
	uint count = (uint)bvh.treelets.size();
	if (type == MESH_TYPE::STATIC)
		count = bvh.static_treelets;
	else if (type == MESH_TYPE::DYNAMIC)
	{
		first = bvh.static_treelets;
		count -= first;
	}
	else if (type != MESH_TYPE::ALL)
		return false;

	PROFILE_GPU(L"scene_bvh_cull");

	auto& compute = list.get_compute();
	auto& copy = list.get_copy();

	if (uploaded != bvh.version)
	{
		PROFILE(L"scene_bvh_upload");
		nodes->reserve(list, bvh.nodes.size());
		treelets->reserve(list, bvh.treelets.size());
		copy.update(nodes->buffer, 0, std::span{ bvh.nodes });
		copy.update(treelets->buffer, 0, std::span{ bvh.treelets });
		uploaded = bvh.version;
	}

	ids->reserve(list, scene.command_ids[(int)MESH_TYPE::ALL].size());

	// Same reset as the other producers of an indirect dispatch (mesh_renderer::generate_boxes).
	compute.clear_counter(ids->buffer);
	compute.clear(args);

	if (count == 0)
		return true;

	Slots::Meshes::SceneBvhCull cull;
	cull.GetNodes() = nodes->buffer.structuredBuffer;
	cull.GetTreelets() = treelets->buffer.structuredBuffer;
	cull.GetTreelet_first() = first;
	cull.GetTreelet_count() = count;
	cull.GetVolume() = volume;
	cull.GetIds() = ids->buffer;
	cull.GetCount() = ids->buffer.counter_view;
	cull.GetArgs() = args;

	compute.set_pipeline<PSOS::Meshes::SceneBvhCull>();
	compute.set(cull);
	compute.dispatch((int)Math::DivideByMultiple(count, 64), 1, 1);
	return true;
}

void Scene::rebuild_scene_caches()
{
	PROFILE(L"scene_rebuild_caches");

	mesh_objects.clear();
	mats.clear();
	pipelines.clear();

	mesh_objects.reserve(static_objects.size() + dynamic_objects.size());

	{
		PROFILE(L"scene_collect_objects");

		// static_cast, not dynamic_cast: the on_element_add handler already
		// filtered these, so every entry is a MeshAssetInstance by construction.
		for (auto o : static_objects)
			mesh_objects.push_back(static_cast<MeshAssetInstance*>(o));
		for (auto o : dynamic_objects)
			mesh_objects.push_back(static_cast<MeshAssetInstance*>(o));
	}

	{
		PROFILE(L"scene_collect_materials");
		for (auto m : mesh_objects)
			for (auto& r : m->rendering)
				mats.insert(r.material);
	}

	{
		// Once per UNIQUE material. Doing this per rendering entry meant a mesh
		// whose submeshes share a material paid a map write for each of them.
		PROFILE(L"scene_collect_pipelines");
		for (auto mat : mats)
		{
			auto um = static_cast<materials::universal_material*>(mat);
			pipelines[um->get_id()] = um->get_pipeline();
		}
	}

	scene_caches_dirty    = false;
	cached_material_epoch = materials::universal_material::pipeline_epoch.load();
}

bool Scene::init_ras(HAL::CommandList::ptr& list)
{
	if (scene_caches_dirty)
		rebuild_scene_caches();

	bool res = false;

	for (auto m : mesh_objects)
		res |= m->init_ras(list);

	return res;
}

void Scene::update(HAL::FrameResources& frame)
{

	  	PROFILE(L"Scene::update");

	{
		PROFILE(L"scene_collect");

		if (scene_caches_dirty
			|| cached_material_epoch != materials::universal_material::pipeline_epoch.load())
		{
			rebuild_scene_caches();
		}
	}

	{
		PROFILE(L"scene_material_update");
		for (auto mat : mats)
		{
			mat->update();
		}
	}


	{
		PROFILE(L"SceneData");
		Slots::Frame::SceneData sceneData;
		sceneData.GetNodes() = universal_nodes_manager::get().buffer;
		sceneData.GetMaterials() = universal_material_info_part_manager::get().buffer;
		sceneData.GetMeshes() = scene->mesh_infos->buffer;
sceneData.GetRaytraceInstanceInfo() = universal_rtx_manager::get().buffer;
		compiledScene = sceneData.compile(frame);
	}


	auto build = [&](my_unique_vector<UINT>& data, Slots::Meshes::GatherPipelineGlobal::Compiled& target) {



		{
			//	auto timer = list.start(L"GatherMat");
			Slots::Meshes::GatherPipelineGlobal gather_global;
			{
				PROFILE(L"gather_count_view");
				//	gather_global.GetMeshes_count() = data.size();


				auto info = frame.place_raw(UINT(data.size()));
				auto srv = info.resource->create_view<HAL::StructuredBufferView<UINT>>(frame, StructuredBufferViewDesc{ (UINT)info.resource_offset, (UINT)info.size,counterType::NONE }).structuredBuffer;
				gather_global.GetMeshes_count() = srv;
			}


			if (data.size()) {
				PROFILE(L"gather_commands_view");
				auto info = frame.place_raw(data);
				auto srv = info.resource->create_view<HAL::StructuredBufferView<UINT>>(frame, StructuredBufferViewDesc{ (UINT)info.resource_offset, (UINT)info.size, counterType::NONE }).structuredBuffer;
				gather_global.GetCommands() = srv;
			}

			{
				PROFILE(L"gather_compile");
				target = gather_global.compile(frame);
			}

		}


	};

	{
		PROFILE(L"scene_gather_static");
		build(command_ids[(int)MESH_TYPE::STATIC], compiledGather[(int)MESH_TYPE::STATIC]);
	}
	{
		PROFILE(L"scene_gather_dynamic");
		build(command_ids[(int)MESH_TYPE::DYNAMIC], compiledGather[(int)MESH_TYPE::DYNAMIC]);
	}
	{
		PROFILE(L"scene_gather_all");
		build(command_ids[(int)MESH_TYPE::ALL], compiledGather[(int)MESH_TYPE::ALL]);
	}


}
