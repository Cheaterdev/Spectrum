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
	void check_mips_match(const TextureStreaming::Streamed& texture, uint32_t first)
	{
		auto readback = read_back(*texture.texture);
		for (uint32_t mip = first; mip < texture.source->mip_maps; mip++)
		{
			const auto& expected = *texture.source->array[0]->mips[mip];
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

// With room for only part of the texture, wanting a finer mip evicts the tiles of finer mips that are no longer wanted,
// and what was evicted comes back when it is wanted again.
TEST(TextureStreaming, EvictsWhatIsNoLongerWanted)
{
	auto& manager = TextureStreaming::Manager::get();
	manager.set_pool_tiles(400);

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
	data.GetWanted() = manager.wanted.rwStructuredBuffer;
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
		data.GetWanted() = manager.wanted.rwStructuredBuffer;
		data.GetReport_level() = reported_mip;
		data.GetView() = view.texture2D.get_offset();
		data.GetUv() = float2(0.5f, 0.5f);
		compute.set(data);

		compute.set_pipeline<PSOS::TextureStreamingTests::StreamClampProbe>();
		compute.dispatch(ivec2(16, 1), ivec2(16, 1));

		uint32_t reported = 0;
		auto pending = list->get_copy().read<uint>(manager.wanted, view.texture2D.get_offset(), 1, [&](std::span<uint> r) { reported = r[0]; });
		list->execute_and_wait();
		pending.wait();

		ASSERT_EQ(16u - reported_mip, reported);
	}

	// Frames the way the engine runs them: requests from what came back, then this frame's collection and tile changes.
	bool streamed = false;
	for (uint32_t frame = 0; frame < 2000 && !streamed; frame++)
	{
		manager.request_from_feedback();

		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"TextureStreaming frame");
		manager.collect_feedback(list);
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

}
