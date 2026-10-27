module;

export module Test.Quanta;

#define TEST_MODULE_ID Quanta

export import Test.Framework;

import Core;
import HAL;
import RenderSystem;
import Graphics;

// Defined in RenderSystem/Quanta/QuantaBuilder.cpp (a plain TU).
extern "C++" Quanta::Mesh BuildQuanta(const uint32_t* indices, uint32_t index_count, const Quanta::Vertex* vertices, uint32_t vertex_count);

// Importing a model creates its materials, which need the graphics system (and leak GPU objects if it is
// never torn down).
SETUP_CATEGORY(Quanta.Import, []() {
	if (!GraphicsSystem::create())
		Test::TestRegistry::Instance().SkipCategory("Quanta.Import", "no suitable GPU device found");
});

TEARDOWN_CATEGORY(Quanta.Import, []() {
	GraphicsSystem::reset();
});

namespace
{
	struct TestMesh
	{
		std::vector<Quanta::Vertex> vertices;
		std::vector<uint32_t>       indices;
	};

	Quanta::Vertex make_vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v)
	{
		Quanta::Vertex out{};
		out.position = float3(x, y, z);
		out.normal = float3(nx, ny, nz);
		out.uv = float2(u, v);
		out.tangent = float4(1, 0, 0, 1);
		return out;
	}

	// A closed mesh, the usual case: every edge has two triangles.
	TestMesh make_sphere(uint32_t slices, uint32_t stacks)
	{
		TestMesh m;
		for (uint32_t y = 0; y <= stacks; y++)
			for (uint32_t x = 0; x <= slices; x++)
			{
				float u = float(x) / slices, v = float(y) / stacks;
				float phi = v * 3.14159265f, theta = u * 2.0f * 3.14159265f;
				float nx = std::sin(phi) * std::cos(theta), ny = std::cos(phi), nz = std::sin(phi) * std::sin(theta);
				m.vertices.push_back(make_vertex(nx, ny, nz, nx, ny, nz, u, v));
			}
		for (uint32_t y = 0; y < stacks; y++)
			for (uint32_t x = 0; x < slices; x++)
			{
				uint32_t a = y * (slices + 1) + x, b = a + 1, c = a + slices + 1, d = c + 1;
				m.indices.insert(m.indices.end(), { a, c, b, b, c, d });
			}
		return m;
	}

	// An open mesh: the border has to stay put through simplification.
	TestMesh make_grid(uint32_t n)
	{
		TestMesh m;
		for (uint32_t y = 0; y <= n; y++)
			for (uint32_t x = 0; x <= n; x++)
				m.vertices.push_back(make_vertex(float(x), 0, float(y), 0, 1, 0, float(x) / n, float(y) / n));
		for (uint32_t y = 0; y < n; y++)
			for (uint32_t x = 0; x < n; x++)
			{
				uint32_t a = y * (n + 1) + x, b = a + 1, c = a + n + 1, d = c + 1;
				m.indices.insert(m.indices.end(), { a, c, b, b, c, d });
			}
		return m;
	}

	Quanta::Mesh build(const TestMesh& m)
	{
		return BuildQuanta(m.indices.data(), static_cast<uint32_t>(m.indices.size()), m.vertices.data(), static_cast<uint32_t>(m.vertices.size()));
	}
}

export namespace Test
{
	TEST(Quanta, SphereIsConsistent)
	{
		TestMesh sphere = make_sphere(256, 128);
		Quanta::Mesh mesh = build(sphere);
		ASSERT_EQ(std::string(), Quanta::validate(mesh));
		ASSERT_EQ(uint32_t(sphere.indices.size() / 3), mesh.original_triangle_count);
	}

	TEST(Quanta, OpenGridIsConsistent)
	{
		TestMesh grid = make_grid(200);
		Quanta::Mesh mesh = build(grid);
		ASSERT_EQ(std::string(), Quanta::validate(mesh));
	}

	TEST(Quanta, HasCoarserLevelsAndRoots)
	{
		Quanta::Mesh mesh = build(make_sphere(256, 128));
		ASSERT_TRUE(mesh.max_depth > 0);
		ASSERT_TRUE(mesh.root_page_count >= 1);
		ASSERT_TRUE(mesh.pages.front().root);
	}

	// The point of paging: what must stay resident is a small part of the whole.
	TEST(Quanta, RootPagesAreASmallPart)
	{
		Quanta::Mesh mesh = build(make_sphere(256, 128));
		ASSERT_TRUE(mesh.pages.size() > 1);

		size_t total = 0, roots = 0;
		for (auto& page : mesh.pages)
		{
			total += page.data.size();
			if (page.root)
				roots += page.data.size();
		}
		ASSERT_TRUE(roots * 4 < total);
	}

	TEST(Quanta, FileRoundTrip)
	{
		Quanta::Mesh mesh = build(make_sphere(128, 64));
		std::vector<uint8_t> bytes = Quanta::serialize(mesh);

		Quanta::Mesh loaded;
		ASSERT_TRUE(Quanta::deserialize(bytes.data(), bytes.size(), loaded));
		ASSERT_EQ(std::string(), Quanta::validate(loaded));

		ASSERT_EQ(mesh.groups.size(), loaded.groups.size());
		ASSERT_TRUE(std::memcmp(mesh.groups.data(), loaded.groups.data(), mesh.groups.size() * sizeof(Quanta::Group)) == 0);
		ASSERT_EQ(mesh.nodes.size(), loaded.nodes.size());
		ASSERT_TRUE(std::memcmp(mesh.nodes.data(), loaded.nodes.data(), mesh.nodes.size() * sizeof(Quanta::Node)) == 0);
		ASSERT_EQ(mesh.pages.size(), loaded.pages.size());
		for (size_t p = 0; p < mesh.pages.size(); p++)
		{
			ASSERT_TRUE(mesh.pages[p].data == loaded.pages[p].data);
			ASSERT_TRUE(mesh.pages[p].groups == loaded.pages[p].groups);
			ASSERT_EQ(mesh.pages[p].root, loaded.pages[p].root);
		}
		ASSERT_EQ(mesh.root_page_count, loaded.root_page_count);
		ASSERT_EQ(mesh.original_triangle_count, loaded.original_triangle_count);
	}

	TEST(Quanta, FileRejectsDamagedInput)
	{
		Quanta::Mesh mesh = build(make_sphere(64, 32));
		std::vector<uint8_t> bytes = Quanta::serialize(mesh);
		Quanta::Mesh out;

		ASSERT_FALSE(Quanta::deserialize(bytes.data(), 0, out));
		ASSERT_FALSE(Quanta::deserialize(bytes.data(), 16, out));
		ASSERT_FALSE(Quanta::deserialize(bytes.data(), bytes.size() / 2, out));
		ASSERT_FALSE(Quanta::deserialize(bytes.data(), bytes.size() - 1, out));

		std::vector<uint8_t> bad_magic = bytes;
		bad_magic[0] ^= 0xFF;
		ASSERT_FALSE(Quanta::deserialize(bad_magic.data(), bad_magic.size(), out));

		std::vector<uint8_t> bad_version = bytes;
		bad_version[4] ^= 0xFF;
		ASSERT_FALSE(Quanta::deserialize(bad_version.data(), bad_version.size(), out));
	}

	// The "quanta" entry of an asset file: next to other entries, with page positions that are real file offsets.
	TEST(Quanta, AssetPartitionRoundTrip)
	{
		const std::filesystem::path path = std::filesystem::temp_directory_path() / "quanta_partition_test.bin";

		Quanta::AssetPartition written;
		written.meshes.push_back(std::make_shared<Quanta::Mesh>(build(make_sphere(64, 32))));
		written.meshes.push_back(std::make_shared<Quanta::Mesh>(build(make_grid(80))));

		{
			FileDataStorage storage(path);
			storage.start_save();
			storage.put("asset", 42);   // something ahead of it, so the offsets are not just near zero
			storage.put("quanta", written);
			storage.save();
		}

		FileDataStorage storage(path);
		ASSERT_TRUE(storage.has("quanta"));

		Quanta::AssetPartition loaded;
		ASSERT_TRUE(storage.get("quanta", loaded));
		ASSERT_EQ(written.meshes.size(), loaded.meshes.size());

		std::ifstream file(path, std::ios::binary);
		for (size_t m = 0; m < written.meshes.size(); m++)
		{
			const Quanta::Mesh& a = *written.meshes[m];
			const Quanta::Mesh& b = *loaded.meshes[m];
			ASSERT_EQ(std::string(), Quanta::validate(b));
			ASSERT_TRUE(b.file == path);
			ASSERT_EQ(a.pages.size(), b.pages.size());

			for (size_t p = 0; p < b.pages.size(); p++)
			{
				ASSERT_TRUE(a.pages[p].data == b.pages[p].data);
				ASSERT_EQ(uint32_t(b.pages[p].data.size()), b.pages[p].file_size);

				// What a DirectStorage request would read: the page's bytes at its offset in the file.
				std::vector<uint8_t> from_file(b.pages[p].file_size);
				file.seekg(b.pages[p].file_offset, std::ios::beg);
				file.read(reinterpret_cast<char*>(from_file.data()), from_file.size());
				ASSERT_TRUE(from_file == b.pages[p].data);
			}
		}
		file.close();
		std::filesystem::remove(path);
	}

	// A file written before Quanta existed has no entry; asking for it must not fail.
	TEST(Quanta, FileWithoutPartitionHasNone)
	{
		const std::filesystem::path path = std::filesystem::temp_directory_path() / "quanta_old_asset_test.bin";
		{
			FileDataStorage storage(path);
			storage.start_save();
			storage.put("asset", 42);
			storage.save();
		}

		FileDataStorage storage(path);
		ASSERT_TRUE(storage.has("asset"));
		ASSERT_FALSE(storage.has("quanta"));
		std::filesystem::remove(path);
	}

	// Resident-only upload: every page and both tables land in the shared GPU buffers where the CPU says they do.
	TEST(Quanta.Import, ResidencyUploadsPagesAndTables)
	{
		auto mesh = std::make_shared<Quanta::Mesh>(build(make_sphere(128, 64)));
		ASSERT_TRUE(mesh->pages.size() >= 1);

		auto& residency = Quanta::Residency::get();
		auto resident = residency.acquire(mesh);
		ASSERT_TRUE(residency.acquire(mesh) == resident);   // same mesh, same copy

		auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"QuantaResidency");
		residency.prepare(list);

		std::vector<uint> page_dwords, group_dwords, node_dwords;
		std::vector<std::future<bool>> pending;
		pending.push_back(list->get_copy().read<uint>(residency.pages.buffer, resident->page_offset(), mesh->pages.size() * Quanta::PageDwords,
			[&](std::span<uint> r) { page_dwords.assign(r.begin(), r.end()); }));
		pending.push_back(list->get_copy().read<uint>(residency.groups.buffer, resident->group_offset() * Quanta::GroupDwords, mesh->groups.size() * Quanta::GroupDwords,
			[&](std::span<uint> r) { group_dwords.assign(r.begin(), r.end()); }));
		pending.push_back(list->get_copy().read<uint>(residency.nodes.buffer, resident->node_offset() * Quanta::NodeDwords, mesh->nodes.size() * Quanta::NodeDwords,
			[&](std::span<uint> r) { node_dwords.assign(r.begin(), r.end()); }));
		list->execute_and_wait();
		for (auto& p : pending)
			p.wait();

		ASSERT_EQ(mesh->pages.size() * Quanta::PageDwords, page_dwords.size());
		for (size_t p = 0; p < mesh->pages.size(); p++)
		{
			const auto& data = mesh->pages[p].data;
			ASSERT_TRUE(std::memcmp(page_dwords.data() + p * Quanta::PageDwords, data.data(), data.size()) == 0);
		}

		// The group records carry the page's dword offset in the shared buffer; the node's groups are global indices.
		ASSERT_EQ(mesh->groups.size() * Quanta::GroupDwords, group_dwords.size());
		for (size_t g = 0; g < mesh->groups.size(); g++)
		{
			Quanta::Group gpu;
			std::memcpy(&gpu, &group_dwords[g * Quanta::GroupDwords], sizeof(gpu));
			ASSERT_EQ(uint32_t(resident->page_offset() + uint64_t(mesh->groups[g].page) * Quanta::PageDwords), gpu.page);
			ASSERT_EQ(mesh->groups[g].offset_in_page, gpu.offset_in_page);
			ASSERT_EQ(mesh->groups[g].lod_error, gpu.lod_error);
		}

		ASSERT_EQ(mesh->nodes.size() * Quanta::NodeDwords, node_dwords.size());
		for (size_t n = 0; n < mesh->nodes.size(); n++)
		{
			Quanta::Node gpu;
			std::memcpy(&gpu, &node_dwords[n * Quanta::NodeDwords], sizeof(gpu));
			ASSERT_EQ(uint32_t(resident->group_offset() + mesh->nodes[n].first_group), gpu.first_group);
			ASSERT_EQ(mesh->nodes[n].group_count, gpu.group_count);
		}
	}

	// MeshAsset writes and reads its Quanta data through the asset-file hooks.
	TEST(Quanta.Import, AssetKeepsQuantaInItsFile)
	{
		const std::filesystem::path path = std::filesystem::temp_directory_path() / "quanta_mesh_asset_test.bin";

		auto asset = std::make_shared<MeshAsset>(L"resources/sphere.obj");
		ASSERT_FALSE(asset->quanta.empty());
		{
			FileDataStorage storage(path);
			storage.start_save();
			asset->save_partitions(storage);
			storage.save();
		}

		auto other = std::make_shared<MeshAsset>(L"resources/sphere.obj");
		other->quanta.clear();
		{
			FileDataStorage storage(path);
			other->load_partitions(storage, nullptr);
		}
		ASSERT_EQ(asset->quanta.size(), other->quanta.size());
		for (size_t i = 0; i < other->quanta.size(); i++)
		{
			ASSERT_EQ(std::string(), Quanta::validate(*other->quanta[i]));
			ASSERT_EQ(asset->quanta[i]->groups.size(), other->quanta[i]->groups.size());
		}

		// An asset file saved before this existed: no entry, nothing loaded, no error.
		{
			FileDataStorage storage(path);
			storage.start_save();
			storage.put("asset", 42);
			storage.save();
		}
		other->quanta.clear();
		{
			FileDataStorage storage(path);
			other->load_partitions(storage, nullptr);
		}
		ASSERT_TRUE(other->quanta.empty());
		std::filesystem::remove(path);
	}

	// The importer builds Quanta next to the meshlets, and the same geometry goes into both.
	TEST(Quanta.Import, ImportBuildsQuantaForEveryMesh)
	{
		// The importer reports progress to the context's task, as MeshAsset's constructor sets up.
		auto context = std::make_shared<AssetLoadingContext>();
		context->loading_task = TaskInfoManager::get().create_task(L"quanta test");
		auto data = MeshData::get_resource("resources/sphere.obj", context);
		ASSERT_TRUE(data != nullptr);
		ASSERT_FALSE(data->meshes.empty());
		ASSERT_EQ(data->meshes.size(), data->quanta.size());

		for (size_t i = 0; i < data->meshes.size(); i++)
		{
			ASSERT_TRUE(data->quanta[i] != nullptr);
			ASSERT_EQ(std::string(), Quanta::validate(*data->quanta[i]));
			ASSERT_EQ(data->meshes[i].index_count / 3, data->quanta[i]->original_triangle_count);
		}
	}

	TEST(Quanta, SameInputSameOutput)
	{
		TestMesh sphere = make_sphere(64, 32);
		Quanta::Mesh a = build(sphere), b = build(sphere);
		ASSERT_EQ(a.groups.size(), b.groups.size());
		ASSERT_EQ(a.pages.size(), b.pages.size());
		for (size_t p = 0; p < a.pages.size(); p++)
			ASSERT_TRUE(a.pages[p].data == b.pages[p].data);
	}
}
