module;

export module Test.TextureStreaming;

#define TEST_MODULE_ID TextureStreamingTests

export import Test.Framework;

import Core;
import HAL;
import RenderSystem;
import Graphics;

SETUP_CATEGORY(TextureStreaming, []() {
	if (!GraphicsSystem::create())
		Test::TestRegistry::Instance().SkipCategory("TextureStreaming", "no suitable GPU device found");
	else if (!TextureStreaming::Manager::supported())
		Test::TestRegistry::Instance().SkipCategory("TextureStreaming", "needs reserved textures (D3D12)");
	else
		TextureStreaming::Manager::set_resident_size(0);   // the tests below start from the coarsest mip alone
});

TEARDOWN_CATEGORY(TextureStreaming, []() {
	GraphicsSystem::reset();
});

namespace
{
	// Every texel says where it is: the mip in red, the position in green and blue, the 256-texel block in alpha.
	HAL::texture_data::ptr make_pattern(uint32_t size, uint32_t mips)
	{
		auto data = std::make_shared<HAL::texture_data>(1, mips, size, size, 1, HAL::Format::R8G8B8A8_UNORM);
		for (uint32_t mip = 0; mip < mips; mip++)
		{
			auto& level = *data->array[0]->mips[mip];
			for (uint32_t y = 0; y < level.height; y++)
				for (uint32_t x = 0; x < level.width; x++)
				{
					uint8_t* texel = &level.data[size_t(y) * level.width_stride + size_t(x) * 4];
					texel[0] = static_cast<uint8_t>(mip * 16 + 1);
					texel[1] = static_cast<uint8_t>(x);
					texel[2] = static_cast<uint8_t>(y);
					texel[3] = static_cast<uint8_t>(((x >> 8) + (y >> 8) * 16) | 1);
				}
		}
		return data;
	}

	using Demand = TextureStreaming::Manager::Demand;

	// Frames until every texture has the mip it was asked for and the streamer has nothing left in flight, with the
	// requests repeated each frame like a draw would.
	bool settle(TextureStreaming::Manager& manager, std::vector<Demand> demands, uint32_t max_frames = 2000)
	{
		for (uint32_t frame = 0; frame < max_frames; frame++)
		{
			manager.request(demands);

			auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
			list->begin(L"TextureStreaming");
			manager.prepare(list);
			list->execute_and_wait();

			bool done = manager.stats().loading_pages == 0;
			for (const Demand& demand : demands)
				done = done && demand.texture->min_mip() <= demand.mip;
			if (done)
				return true;

			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
		return false;
	}

	// Texture::get_data() reads on the copy queue, which cannot take a texture out of its shader-resource resting layout:
	// a streamed texture rests there for good, so it is read back on the direct queue.
	HAL::texture_data::ptr read_back(HAL::Texture& texture)
	{
		const auto desc = texture.get_desc().as_texture();
		auto data = std::make_shared<HAL::texture_data>(1, desc.MipLevels, desc.Dimensions.x, desc.Dimensions.y, 1, desc.Format);

		auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming readback");

		std::vector<std::future<bool>> pending;
		for (uint32_t mip = 0; mip < desc.MipLevels; mip++)
		{
			auto& level = *data->array[0]->mips[mip];
			pending.push_back(list->get_copy().read_texture(texture.resource, ivec3(0, 0, 0), ivec3(int(level.width), int(level.height), 1), mip,
				[&level](std::span<std::byte> memory, HAL::texture_layout layout)
				{
					for (uint32_t row = 0; row < level.num_rows; row++)
						std::memcpy(level.data.data() + size_t(row) * level.width_stride, memory.data() + size_t(row) * layout.row_stride, level.width_stride);
				}));
		}

		list->execute_and_wait();
		for (auto& p : pending)
			p.wait();
		return data;
	}

	// The mips from `first` down must match the source texel for texel: what the GPU holds is what the file said.
	void check_mips_match(const TextureStreaming::Streamed& texture, uint32_t first, const HAL::texture_data* reference = nullptr)
	{
		const HAL::texture_data& source = reference ? *reference : *texture.source;
		auto readback = read_back(*texture.texture);
		for (uint32_t mip = first; mip < source.mip_maps; mip++)
		{
			const auto& expected = *source.array[0]->mips[mip];
			const auto& actual = *readback->array[0]->mips[mip];
			ASSERT_EQ(expected.data.size(), actual.data.size());
			ASSERT_TRUE(std::memcmp(expected.data.data(), actual.data.data(), expected.data.size()) == 0);
		}
	}
}

export namespace Test
{

// Only the coarsest mip and the mips that do not fill a tile are there at first; asking for finer mips streams them in,
// and every texel that lands on the GPU is the one the source holds.
TEST(TextureStreaming, MipsStreamInAndMatchTheSource)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	auto texture = manager.create(make_pattern(2048, 12), L"pattern");
	ASSERT_EQ(5u, texture->standard_mips());   // 2048 .. 128: a 128x128 mip is one tile of RGBA8
	ASSERT_EQ(texture->standard_mips() - 1, texture->min_mip());

	// Prepared once, the roots and the packed tail are on the GPU.
	{
		auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming");
		manager.prepare(list);
		list->execute_and_wait();
	}
	check_mips_match(*texture, texture->standard_mips() - 1);

	ASSERT_TRUE(settle(manager, { { texture.get(), 2 } }));
	check_mips_match(*texture, 2);

	ASSERT_TRUE(settle(manager, { { texture.get(), 1 } }));
	check_mips_match(*texture, 1);
}

// The resident size: every mip up to it is loaded when the texture is made and is not a candidate for eviction, so
// whatever samples it (ray tracing reports nothing) always finds at least that much.
TEST(TextureStreaming, KeepsTheResidentSizeLoaded)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	manager.set_resident_size(512);
	auto texture = manager.create(make_pattern(2048, 12), L"pattern");
	manager.set_resident_size(0);

	ASSERT_EQ(5u, texture->standard_mips());       // 2048 .. 128
	ASSERT_EQ(2u, texture->resident_floor_mip());   // 512 is the finest mip that fits
	ASSERT_EQ(2u, texture->min_mip());

	{
		auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming");
		manager.prepare(list);
		list->execute_and_wait();
	}
	check_mips_match(*texture, 2);

	ASSERT_TRUE(settle(manager, { { texture.get(), 1 } }));
	check_mips_match(*texture, 1);
}

// With room for only part of the texture, wanting a finer mip evicts the tiles of finer mips that are no longer wanted,
// and what was evicted comes back when it is wanted again.
TEST(TextureStreaming, EvictsWhatIsNoLongerWanted)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(398);   // 400 tiles with the two textures' roots, which are outside the budget

	auto texture = manager.create(make_pattern(2048, 12), L"pattern");
	ASSERT_TRUE(settle(manager, { { texture.get(), 1 } }));   // 64 + 16 + 4 + 1 tiles

	const uint64_t evictions_before = manager.stats().evictions;

	// Back to a coarse mip, then the pool is too small for anything else, so mip 1 tiles must go for a second texture.
	auto other = manager.create(make_pattern(2048, 12), L"other");
	ASSERT_TRUE(settle(manager, { { texture.get(), 3 } }));
	ASSERT_TRUE(settle(manager, { { texture.get(), 3 }, { other.get(), 0 } }));

	ASSERT_TRUE(manager.stats().evictions > evictions_before);
	ASSERT_TRUE(settle(manager, { { other.get(), 3 }, { texture.get(), 1 } }));
	check_mips_match(*texture, 1);
	check_mips_match(*other, 3);
}

// A pool lowered under what is resident gives tiles back even though the draw still wants them: the quality drops to what fits.
TEST(TextureStreaming, ShrinkingThePoolEvictsWantedTiles)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	auto texture = manager.create(make_pattern(2048, 12), L"pattern");
	ASSERT_TRUE(settle(manager, { { texture.get(), 1 } }));   // 64 + 16 + 4 + 1 tiles
	ASSERT_EQ(1u, texture->min_mip());

	const uint64_t evictions_before = manager.stats().evictions;
	const uint32_t pool_before = manager.stats().pool_pages;
	manager.set_pool_tiles(9);   // 10 tiles with the one root, which is outside the budget

	for (uint32_t frame = 0; frame < 200; frame++)
	{
		std::vector<Demand> demands = { { texture.get(), 1 } };
		manager.request(demands);

		auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming");
		manager.prepare(list);
		list->execute_and_wait();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	ASSERT_TRUE(manager.stats().evictions > evictions_before);
	ASSERT_TRUE(texture->min_mip() > 1);

	// The memory of the evicted tiles went back to the heap, not only their budget.
	ASSERT_TRUE(manager.stats().pool_pages < pool_before);

	// And a larger pool takes memory again, as the tiles load.
	manager.set_pool_tiles(400);
	ASSERT_TRUE(settle(manager, { { texture.get(), 1 } }));
	check_mips_match(*texture, 1);
	ASSERT_TRUE(manager.stats().pool_pages > 10);
}

// Nothing is allocated for a pool nobody uses: the memory follows what is loaded, in chunks, and stays far under a big budget.
TEST(TextureStreaming, PoolMemoryFollowsWhatIsLoaded)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(4000);

	auto texture = manager.create(make_pattern(2048, 12), L"pattern");
	ASSERT_TRUE(settle(manager, { { texture.get(), 1 } }));   // 64 + 16 + 4 + 1 tiles
	ASSERT_TRUE(manager.stats().pool_pages < 400);

	manager.set_pool_tiles(400);
}

// The clamp the material shaders apply: with only the coarse mips resident, sampling a finer level returns the finest
// resident one, whether the level comes from an explicit LOD or from gradients.
TEST(TextureStreaming, ShaderClampsToTheResidentMip)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	constexpr uint32_t resident_mip = 2;
	auto texture = manager.create(make_pattern(2048, 12), L"pattern");
	ASSERT_TRUE(settle(manager, { { texture.get(), resident_mip } }));
	ASSERT_EQ(resident_mip, texture->min_mip());

	auto& device = RenderSystem::get().device();
	HAL::StructuredBufferView<uint> result(device, 32, HAL::counterType::NONE,
		HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);

	auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
	list->begin(L"TextureStreaming clamp probe");

	auto& compute = list->get_compute();
	compute.set_signature(Layouts::DefaultLayout);

	auto view = texture->texture->resource->create_view<HAL::Texture2DView>(compute.get_base());
	manager.bind_view(texture->texture->resource.get(), view.texture2D.get_offset());
	manager.prepare(list);   // publishes the value for the new view

	list->clear_uav(result.rwRAW, vec4(0, 0, 0, 0));

	Slots::TextureStreamingTests::StreamClampProbe data;
	data.GetSource() = view.texture2D;
	data.GetStream_info() = manager.stream_info.structuredBuffer;
	data.GetResult() = result.rwStructuredBuffer;
	data.GetWanted() = manager.feedback().rwStructuredBuffer;
	data.GetEpoch() = manager.epoch();
	data.GetReport_level() = 100;   // no thread reports
	data.GetView() = view.texture2D.get_offset();
	data.GetUv() = float2(0.37f, 0.61f);
	compute.set(data);

	compute.set_pipeline<PSOS::TextureStreamingTests::StreamClampProbe>();
	compute.dispatch(ivec2(16, 1), ivec2(16, 1));

	std::vector<uint> values;
	auto pending = list->get_copy().read<uint>(result, 0, 32, [&](std::span<uint> r) { values.assign(r.begin(), r.end()); });
	list->execute_and_wait();
	pending.wait();

	ASSERT_EQ(size_t(32), values.size());
	for (uint32_t level = 0; level < 16; level++)
	{
		// The pattern stores mip * 16 + 1 in red, and the hardware stops at the last of the 12 mips.
		const uint32_t mip = std::min(std::max(level, resident_mip), 11u);
		const uint expected = mip * 16 + 1;
		if (values[level * 2] != expected || values[level * 2 + 1] != expected)
			throw Test::TestFailure("level " + std::to_string(level) + ": expected " + std::to_string(expected)
				+ ", SampleLevel " + std::to_string(values[level * 2]) + ", SampleGrad " + std::to_string(values[level * 2 + 1]));
	}
}

// A large texture asset hands the materials a streamed copy of itself, and the asset's own texture stays whole for saving.
TEST(TextureStreaming, AssetGivesMaterialsAStreamedCopy)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	auto asset = std::make_shared<TextureAsset>(make_pattern(2048, 12), L"streamed asset");
	auto whole = asset->get_texture();
	auto render = asset->get_render_texture();

	ASSERT_TRUE(whole != render);
	ASSERT_TRUE(!whole->get_desc().is_virtual());
	ASSERT_TRUE(render->get_desc().is_virtual());
	ASSERT_TRUE(render == asset->get_render_texture());   // made once

	// The roots and the packed tail reach the GPU with the first prepare.
	{
		auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming");
		manager.prepare(list);
		list->execute_and_wait();
	}

	auto expected = make_pattern(2048, 12);
	auto actual = read_back(*render);
	for (uint32_t mip = 4; mip < 12; mip++)
	{
		const auto& e = *expected->array[0]->mips[mip];
		const auto& a = *actual->array[0]->mips[mip];
		ASSERT_EQ(e.data.size(), a.data.size());
		ASSERT_TRUE(std::memcmp(e.data.data(), a.data.data(), e.data.size()) == 0);
	}

	// A texture too small to be worth streaming is used as it is.
	auto small = std::make_shared<TextureAsset>(make_pattern(256, 9), L"small asset");
	ASSERT_TRUE(small->get_render_texture() == small->get_texture());
}

// The whole feedback loop: a shader reports the mip it sampled (stream_report, the helper the material pixel shaders
// use), the report is read back and cleared, and the streamer brings in the mips the report asked for.
TEST(TextureStreaming, ReportedMipsAreStreamedIn)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	constexpr uint32_t reported_mip = 1;
	auto texture = manager.create(make_pattern(2048, 12), L"pattern");
	ASSERT_EQ(texture->standard_mips() - 1, texture->min_mip());

	auto& device = RenderSystem::get().device();
	HAL::StructuredBufferView<uint> result(device, 32, HAL::counterType::NONE,
		HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);

	// Three levels from `reported_mip` report; the finest of them is what must come out.
	{
		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming report");

		auto& compute = list->get_compute();
		compute.set_signature(Layouts::DefaultLayout);

		auto view = texture->texture->resource->create_view<HAL::Texture2DView>(compute.get_base());
		manager.bind_view(texture->texture->resource.get(), view.texture2D.get_offset());
		manager.prepare(list);   // the view's stream_info entry: a texture that is not streamed reports nothing

		Slots::TextureStreamingTests::StreamClampProbe data;
		data.GetSource() = view.texture2D;
		data.GetStream_info() = manager.stream_info.structuredBuffer;
		data.GetResult() = result.rwStructuredBuffer;
		data.GetWanted() = manager.feedback().rwStructuredBuffer;
		data.GetEpoch() = manager.epoch();
		data.GetReport_level() = reported_mip;
		data.GetView() = view.texture2D.get_offset();
		data.GetUv() = float2(0.5f, 0.5f);
		compute.set(data);

		compute.set_pipeline<PSOS::TextureStreamingTests::StreamClampProbe>();
		compute.dispatch(ivec2(16, 1), ivec2(16, 1));

		// One id per mip of the texture: each of the three levels stamped its own, with this frame's epoch.
		uint32_t stamps[6] = {};
		auto pending = list->get_copy().read<uint>(manager.feedback(), uint64_t(texture->mip_id()) * 2 + reported_mip * 2, 6,
			[&](std::span<uint> r) { std::copy(r.begin(), r.end(), stamps); });
		const uint32_t epoch = manager.epoch();
		list->execute_and_wait();
		pending.wait();

		for (uint32_t i = 0; i < 3; i++)
		{
			ASSERT_EQ(epoch & 0xFFFFu, stamps[i * 2] >> 16);
			ASSERT_TRUE((stamps[i * 2] & 0xFFFFu) != 0);
			ASSERT_EQ(epoch, stamps[i * 2 + 1]);
		}
		// Coarser mips are more urgent.
		ASSERT_TRUE((stamps[2] & 0xFFFFu) > (stamps[0] & 0xFFFFu));
	}

	// Frames the way the engine runs them: each prepare compacts the previous frame's report and turns it into requests.
	bool streamed = false;
	for (uint32_t frame = 0; frame < 2000 && !streamed; frame++)
	{
		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming frame");
		manager.prepare(list);
		list->execute_and_wait();

		streamed = manager.stats().loading_pages == 0 && texture->min_mip() <= reported_mip;
		if (!streamed)
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}

	ASSERT_TRUE(streamed);
	ASSERT_EQ(reported_mip, texture->min_mip());
	check_mips_match(*texture, reported_mip);
}

// The tiles of a texture baked into an asset-file partition and streamed from there: nothing of the texture is kept in
// memory, each tile is read from the file when it is wanted, and every texel that lands on the GPU is the original one.
TEST(TextureStreaming, TilesStreamFromAnAssetFile)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	auto data = make_pattern(2048, 12);
	auto baked = manager.bake(data);
	ASSERT_EQ(5u, static_cast<uint32_t>(baked->mips.size()));
	ASSERT_EQ(7u, static_cast<uint32_t>(baked->packed.size()));   // 64 .. 1

	const auto path = std::filesystem::temp_directory_path() / "spectrum_texture_tiles_test.temp";
	{
		FileDataStorage storage(path);
		storage.start_save();
		storage.put("asset", 42);   // something ahead of it, so the offsets are not near zero
		storage.put("tiles", *baked);
		storage.save();
	}
	baked.reset();

	auto file = std::make_shared<TextureStreaming::TileFile>();
	{
		FileDataStorage storage(path);
		ASSERT_TRUE(storage.has("tiles"));
		ASSERT_TRUE(storage.get("tiles", *file));
	}
	ASSERT_TRUE(file->tile_bytes.empty());   // the bytes stayed in the file

	auto texture = manager.create(file, L"from file");
	ASSERT_TRUE(texture->source == nullptr);
	ASSERT_EQ(texture->standard_mips() - 1, texture->min_mip());

	{
		auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming");
		manager.prepare(list);
		list->execute_and_wait();
	}
	check_mips_match(*texture, texture->standard_mips() - 1, data.get());

	ASSERT_TRUE(settle(manager, { { texture.get(), 1 } }));
	check_mips_match(*texture, 1, data.get());

	ASSERT_TRUE(settle(manager, { { texture.get(), 0 } }));
	check_mips_match(*texture, 0, data.get());

	// The tiles came through DirectStorage, mapped by start_loads, not cut on the CPU.
	ASSERT_TRUE(manager.stats().file_loads > 0);

	texture.reset();
	file.reset();
	std::filesystem::remove(path);
}

// A streamable asset is saved as its tiles and loads as a streamed texture with no whole texture behind it; the whole one
// is made from the texels in the file only when something asks for it. A texture that must not stream is saved whole.
TEST(TextureStreaming, StreamableAssetSavesAsTiles)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	const auto path = std::filesystem::temp_directory_path() / "spectrum_texture_asset_test.temp";

	auto data = make_pattern(2048, 12);
	auto save_and_load = [&](std::shared_ptr<TextureAsset> asset) -> std::shared_ptr<TextureAsset>
	{
		asset->before_save();
		{
			FileDataStorage storage(path);
			storage.start_save();
			storage.put("asset", std::static_pointer_cast<Asset>(asset));
			asset->save_partitions(storage);
			storage.save();
			asset->after_save(storage);
		}

		UniversalContext context;
		context.get_context<HAL::Device*>() = &RenderSystem::get().device();

		FileDataStorage storage(path);
		Asset::ptr loaded;
		ASSERT_TRUE(storage.get("asset", loaded, &context));
		loaded->load_partitions(storage, &context);
		return std::dynamic_pointer_cast<TextureAsset>(loaded);
	};

	// Streamable: loads as a streamed texture.
	{
		auto saved = std::make_shared<TextureAsset>(data, L"saved asset");
		ASSERT_TRUE(saved->is_streamable());
		auto loaded = save_and_load(saved);
		ASSERT_TRUE(loaded != nullptr);
		ASSERT_TRUE(loaded->is_streamable());

		auto render = loaded->get_render_texture();
		ASSERT_TRUE(render->get_desc().is_virtual());

		{
			auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
			list->begin(L"TextureStreaming");
			manager.prepare(list);
			list->execute_and_wait();
		}
		auto actual = read_back(*render);
		for (uint32_t mip = 4; mip < 12; mip++)
		{
			const auto& e = *data->array[0]->mips[mip];
			const auto& a = *actual->array[0]->mips[mip];
			ASSERT_TRUE(std::memcmp(e.data.data(), a.data.data(), e.data.size()) == 0);
		}

		// The whole texture comes from the texels kept in the file, and is the original.
		auto whole = loaded->get_texture();
		ASSERT_TRUE(whole && !whole->get_desc().is_virtual());
		auto whole_data = read_back(*whole);
		for (uint32_t mip = 0; mip < 12; mip++)
		{
			const auto& e = *data->array[0]->mips[mip];
			const auto& a = *whole_data->array[0]->mips[mip];
			ASSERT_TRUE(std::memcmp(e.data.data(), a.data.data(), e.data.size()) == 0);
		}

		// Saved again from what it was loaded as: the file is read before it is written over.
		auto again = save_and_load(loaded);
		ASSERT_TRUE(again->get_render_texture()->get_desc().is_virtual());
	}

	// Not streamable: saved whole, and loads whole.
	{
		auto saved = std::make_shared<TextureAsset>(data, L"whole asset");
		saved->set_streamable(false);
		ASSERT_TRUE(saved->get_render_texture() == saved->get_texture());

		auto loaded = save_and_load(saved);
		ASSERT_TRUE(loaded != nullptr);
		ASSERT_FALSE(loaded->is_streamable());
		ASSERT_TRUE(!loaded->get_render_texture()->get_desc().is_virtual());
		ASSERT_TRUE(loaded->get_render_texture() == loaded->get_texture());
	}

	std::filesystem::remove(path);
}

// A tile read back with CopyTiles and put in a file is what DirectStorage's tile destination loads into another texture:
// the file is the only copy of the texels, with nothing re-cut from a CPU mip.
TEST(TextureStreaming, TileFileLoadsThroughDirectStorage)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

	constexpr uint32_t Size = 1024;   // RGBA8: a tile is 128x128 texels, mip 0 has 8x8 tiles
	constexpr uint32_t Tile = 128;

	auto source = manager.create(make_pattern(Size, 11), L"tile source");

	// The same texture with the position texels mirrored, so a tile that did not arrive shows.
	auto other_data = make_pattern(Size, 11);
	for (uint32_t mip = 0; mip < 11; mip++)
	{
		auto& level = *other_data->array[0]->mips[mip];
		for (uint32_t y = 0; y < level.height; y++)
			for (uint32_t x = 0; x < level.width; x++)
				level.data[size_t(y) * level.width_stride + size_t(x) * 4 + 1] = static_cast<uint8_t>(255 - x);
	}
	auto target = manager.create(other_data, L"tile target");

	ASSERT_TRUE(settle(manager, { { source.get(), 0 }, { target.get(), 0 } }));

	const uint32_t tile_x = 3, tile_y = 2;

	// Read the tile of the source.
	std::vector<std::byte> tile_bytes;
	{
		auto list = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming tile read");
		auto done = list->get_copy().read_tiles(source->texture->resource.get(), uint3(tile_x, tile_y, 0), uint3(1, 1, 1), 0,
			[&tile_bytes](std::span<std::byte> memory) { tile_bytes.assign(memory.begin(), memory.end()); });
		list->execute_and_wait();
		done.wait();
	}
	ASSERT_EQ(size_t(TextureStreaming::TileBytes), tile_bytes.size());

	const auto path = std::filesystem::temp_directory_path() / "spectrum_tile_test.temp";
	{
		std::ofstream file(path, std::ios::binary);
		file.write(reinterpret_cast<const char*>(tile_bytes.data()), std::streamsize(tile_bytes.size()));
	}

	// Load it into the same tile of the target.
	{
		HAL::StorageRequest request;
		request.resource = target->texture->resource;
		request.file = path;
		request.file_offset = 0;
		request.size = TextureStreaming::TileBytes;
		request.uncompressed_size = TextureStreaming::TileBytes;
		request.compressed = false;
		request.operation = HAL::StorageRequest::Tiles{ 0, uint3(tile_x, tile_y, 0), uint3(1, 1, 1) };

		auto& queue = RenderSystem::get().device().get_ds_queue();
		auto waiter = queue.execute(request);
		queue.flush();
		waiter.wait();
	}

	auto readback = read_back(*target->texture);
	const auto& level = *readback->array[0]->mips[0];
	const auto& expected = *source->source->array[0]->mips[0];

	for (uint32_t y = 0; y < Tile; y++)
		ASSERT_TRUE(std::memcmp(level.data.data() + (size_t(tile_y) * Tile + y) * level.width_stride + size_t(tile_x) * Tile * 4,
			expected.data.data() + (size_t(tile_y) * Tile + y) * expected.width_stride + size_t(tile_x) * Tile * 4, size_t(Tile) * 4) == 0);

	// A neighbouring tile is still the target's own.
	ASSERT_TRUE(std::memcmp(level.data.data() + (size_t(tile_y) * Tile) * level.width_stride + size_t(tile_x + 1) * Tile * 4,
		target->source->array[0]->mips[0]->data.data() + (size_t(tile_y) * Tile) * level.width_stride + size_t(tile_x + 1) * Tile * 4, size_t(Tile) * 4) == 0);

	std::filesystem::remove(path);
}

}
