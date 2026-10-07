export module Test.HAL;

#define TEST_MODULE_ID HAL

export import Test.Framework;
export import Test.HAL.TextureUtils;
import Test.HAL.Rendering;
import Test.HAL.SIG;
import Test.HAL.UI;
import Test.GUI;
import Test.FrameGraph;
import Test.RTX;

import Core;
import HAL;
import Graphics;
import GUI;
SETUP_CATEGORY(Core.HAL, []() {
	if (!GraphicsSystem::create())
		Test::TestRegistry::Instance().SkipCategory("Core.HAL", "no suitable GPU device found");
});

TEARDOWN_CATEGORY(Core.HAL, []() {
	GraphicsSystem::reset();
});

export namespace Test
{
	// Device tests
	TEST(Core.HAL, DeviceCreation)
	{
		ASSERT_TRUE(&RenderSystem::get().device() != nullptr);
	}

	TEST(Core.HAL, DeviceProperties)
	{
		const auto& props = RenderSystem::get().device().get_properties();
		ASSERT_TRUE(!props.name.empty());
	}

	TEST(Core.HAL, DeviceVRAM)
	{
		size_t vram = RenderSystem::get().device().get_vram();
		ASSERT_TRUE(vram > 0);
	}

	// Queue tests
	TEST(Core.HAL, GetDirectQueue)
	{
		auto& queue = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT);
		ASSERT_TRUE(queue != nullptr);
	}

	TEST(Core.HAL, GetComputeQueue)
	{
		auto& queue = RenderSystem::get().device().get_queue(HAL::CommandListType::COMPUTE);
		ASSERT_TRUE(queue != nullptr);
	}

	TEST(Core.HAL, GetCopyQueue)
	{
		auto& queue = RenderSystem::get().device().get_queue(HAL::CommandListType::COPY);
		ASSERT_TRUE(queue != nullptr);
	}

	TEST(Core.HAL, QueueSignalAndWait)
	{
		auto& queue = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT);
		queue->signal_and_wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, QueueSignal)
	{
		auto& queue = RenderSystem::get().device().get_queue(HAL::CommandListType::DIRECT);
		auto waiter = queue->signal();
		waiter.wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, ComputeQueueSignalAndWait)
	{
		auto& queue = RenderSystem::get().device().get_queue(HAL::CommandListType::COMPUTE);
		queue->signal_and_wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, CopyQueueSignalAndWait)
	{
		auto& queue = RenderSystem::get().device().get_queue(HAL::CommandListType::COPY);
		queue->signal_and_wait();
		ASSERT_TRUE(true);
	}

	// Buffer creation tests
	TEST(Core.HAL, CreateUploadBuffer)
	{
		auto& device = RenderSystem::get().device();
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256, HAL::ResFlags::None),
			HAL::HeapType::UPLOAD);
		ASSERT_TRUE(buffer != nullptr);
	}

	TEST(Core.HAL, CreateDefaultBuffer)
	{
		auto& device = RenderSystem::get().device();
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(buffer != nullptr);
	}

	TEST(Core.HAL, CreateReadbackBuffer)
	{
		auto& device = RenderSystem::get().device();
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256, HAL::ResFlags::None),
			HAL::HeapType::READBACK);
		ASSERT_TRUE(buffer != nullptr);
	}

	TEST(Core.HAL, CreateLargeBuffer)
	{
		auto& device = RenderSystem::get().device();
		constexpr size_t SIZE = 1024 * 1024; // 1 MB
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(buffer != nullptr);
		ASSERT_TRUE(buffer->get_size() >= SIZE);
	}

	TEST(Core.HAL, UploadBufferCPUAccess)
	{
		auto& device = RenderSystem::get().device();
		constexpr size_t SIZE = 256;
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE, HAL::ResFlags::None),
			HAL::HeapType::UPLOAD);
		auto data = buffer->cpu_data();
		ASSERT_TRUE(data.data() != nullptr);
		ASSERT_TRUE(data.size() >= SIZE);
	}

	TEST(Core.HAL, ReadbackBufferCPUAccess)
	{
		auto& device = RenderSystem::get().device();
		constexpr size_t SIZE = 256;
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE, HAL::ResFlags::None),
			HAL::HeapType::READBACK);
		auto data = buffer->cpu_data();
		ASSERT_TRUE(data.data() != nullptr);
	}

	TEST(Core.HAL, BufferHeapType)
	{
		auto& device = RenderSystem::get().device();
		auto upload = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256, HAL::ResFlags::None),
			HAL::HeapType::UPLOAD);
		auto def = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256),
			HAL::HeapType::DEFAULT);
		auto readback = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256, HAL::ResFlags::None),
			HAL::HeapType::READBACK);

		ASSERT_TRUE(upload->get_heap_type() == HAL::HeapType::UPLOAD);
		ASSERT_TRUE(def->get_heap_type() == HAL::HeapType::DEFAULT);
		ASSERT_TRUE(readback->get_heap_type() == HAL::HeapType::READBACK);
	}

	TEST(Core.HAL, BufferResourceDesc)
	{
		auto& device = RenderSystem::get().device();
		constexpr size_t SIZE = 512;
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(buffer->get_desc().is_buffer());
		ASSERT_TRUE(buffer->get_desc().as_buffer().SizeInBytes == SIZE);
	}

	// Texture creation tests
	TEST(Core.HAL, CreateTexture2D)
	{
		auto& device = RenderSystem::get().device();
		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {64, 64}, 1, 1),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(tex != nullptr);
	}

	TEST(Core.HAL, CreateTexture2DUAV)
	{
		auto& device = RenderSystem::get().device();
		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {64, 64}, 1, 1,
				HAL::ResFlags::UnorderedAccess | HAL::ResFlags::ShaderResource),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(tex != nullptr);
	}

	TEST(Core.HAL, TextureResourceDesc)
	{
		auto& device = RenderSystem::get().device();
		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R16G16B16A16_FLOAT, {128, 128}, 1, 1),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(tex->get_desc().is_texture());
		const auto& tdesc = tex->get_desc().as_texture();
		ASSERT_TRUE(tdesc.Dimensions.x == 128);
		ASSERT_TRUE(tdesc.Dimensions.y == 128);
		ASSERT_TRUE(tdesc.Format == HAL::Format::R16G16B16A16_FLOAT);
	}

	// Command list tests
	TEST(Core.HAL, ExecuteEmptyDirectCommandList)
	{
		auto& device = RenderSystem::get().device();
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"EmptyDirect");
		list->execute_and_wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, ExecuteEmptyCopyCommandList)
	{
		auto& device = RenderSystem::get().device();
		auto& queue = device.get_queue(HAL::CommandListType::COPY);
		auto list = queue->get_free_list();
		list->begin(L"EmptyCopy");
		list->execute_and_wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, ExecuteMultipleCommandLists)
	{
		auto& device = RenderSystem::get().device();
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);

		for (int i = 0; i < 3; ++i)
		{
			auto list = queue->get_free_list();
			list->begin(L"MultipleTest");
			list->execute_and_wait();
		}
		ASSERT_TRUE(true);
	}

	// Data upload/copy/readback round-trip
	TEST(Core.HAL, WriteToUploadBuffer)
	{
		auto& device = RenderSystem::get().device();
		constexpr size_t SIZE = 64;
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE, HAL::ResFlags::None),
			HAL::HeapType::UPLOAD);

		auto data = buffer->cpu_data();
		for (size_t i = 0; i < SIZE && i < data.size(); ++i)
			data[i] = static_cast<std::byte>(i & 0xFF);

		bool match = true;
		for (size_t i = 0; i < SIZE && i < data.size(); ++i)
		{
			if (data[i] != static_cast<std::byte>(i & 0xFF))
			{
				match = false;
				break;
			}
		}
		ASSERT_TRUE(match);
	}

	TEST(Core.HAL, BufferCopyUploadToReadback)
	{
		auto& device = RenderSystem::get().device();
		constexpr size_t SIZE = 256;

		auto upload = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE, HAL::ResFlags::None),
			HAL::HeapType::UPLOAD);
		auto readback = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE, HAL::ResFlags::None),
			HAL::HeapType::READBACK);

		auto src = upload->cpu_data();
		for (size_t i = 0; i < SIZE && i < src.size(); ++i)
			src[i] = static_cast<std::byte>((i * 3 + 7) & 0xFF);

		auto list = device.get_upload_list();
		list->get_copy().copy_buffer(readback.get(), 0, upload.get(), 0, SIZE);
		list->execute_and_wait();

		auto dst = readback->cpu_data();
		bool match = true;
		for (size_t i = 0; i < SIZE && i < dst.size(); ++i)
		{
			if (dst[i] != static_cast<std::byte>((i * 3 + 7) & 0xFF))
			{
				match = false;
				break;
			}
		}
		ASSERT_TRUE(match);
	}

	TEST(Core.HAL, UploadListHelperPattern)
	{
		auto& device = RenderSystem::get().device();
		constexpr size_t SIZE = 128;

		auto upload = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE, HAL::ResFlags::None),
			HAL::HeapType::UPLOAD);
		auto readback = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE, HAL::ResFlags::None),
			HAL::HeapType::READBACK);

		auto src = upload->cpu_data();
		std::fill(src.begin(), src.begin() + SIZE, static_cast<std::byte>(0xAB));

		auto list = device.get_upload_list();
		list->get_copy().copy_buffer(readback.get(), 0, upload.get(), 0, SIZE);
		list->execute_and_wait();

		auto dst = readback->cpu_data();
		ASSERT_TRUE(dst[0] == static_cast<std::byte>(0xAB));
		ASSERT_TRUE(dst[SIZE - 1] == static_cast<std::byte>(0xAB));
	}

	// Texture content tests
	TEST(Core.HAL, RainbowTexture)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = 256;
		constexpr uint HEIGHT = 64;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1),
			HAL::HeapType::DEFAULT);

		// Generate rainbow pixels (hue varies with X)
		constexpr uint ROW_STRIDE = WIDTH * 4;
		std::vector<uint8_t> pixels(WIDTH * HEIGHT * 4);

		for (uint y = 0; y < HEIGHT; ++y)
		{
			for (uint x = 0; x < WIDTH; ++x)
			{
				float h = (float)x / WIDTH * 360.0f;
				float c = 1.0f;
				float xv = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
				float r = 0, g = 0, b = 0;
				int hi = (int)(h / 60.0f) % 6;
				switch (hi)
				{
				case 0: r = c;  g = xv; b = 0;  break;
				case 1: r = xv; g = c;  b = 0;  break;
				case 2: r = 0;  g = c;  b = xv; break;
				case 3: r = 0;  g = xv; b = c;  break;
				case 4: r = xv; g = 0;  b = c;  break;
				default: r = c; g = 0;  b = xv; break;
				}

				size_t idx = ((size_t)y * WIDTH + x) * 4;
				pixels[idx + 0] = (uint8_t)(r * 255.0f + 0.5f);
				pixels[idx + 1] = (uint8_t)(g * 255.0f + 0.5f);
				pixels[idx + 2] = (uint8_t)(b * 255.0f + 0.5f);
				pixels[idx + 3] = 255;
			}
		}

		auto list = device.get_upload_list();
		list->get_copy().update_texture(tex.get(), ivec3(0, 0, 0), ivec3(WIDTH, HEIGHT, 1), 0,
			reinterpret_cast<const char*>(pixels.data()), ROW_STRIDE);
		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "rainbow");
	}

	TEST(Core.HAL, VerticalRainbowTexture)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = 64;
		constexpr uint HEIGHT = 256;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1),
			HAL::HeapType::DEFAULT);

		// Generate rainbow pixels (hue varies with Y)
		constexpr uint ROW_STRIDE = WIDTH * 4;
		std::vector<uint8_t> pixels(WIDTH * HEIGHT * 4);

		for (uint y = 0; y < HEIGHT; ++y)
		{
			float h = (float)y / HEIGHT * 360.0f;
			float c = 1.0f;
			float xv = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
			float r = 0, g = 0, b = 0;
			int hi = (int)(h / 60.0f) % 6;
			switch (hi)
			{
			case 0: r = c;  g = xv; b = 0;  break;
			case 1: r = xv; g = c;  b = 0;  break;
			case 2: r = 0;  g = c;  b = xv; break;
			case 3: r = 0;  g = xv; b = c;  break;
			case 4: r = xv; g = 0;  b = c;  break;
			default: r = c; g = 0;  b = xv; break;
			}

			uint8_t pr = (uint8_t)(r * 255.0f + 0.5f);
			uint8_t pg = (uint8_t)(g * 255.0f + 0.5f);
			uint8_t pb = (uint8_t)(b * 255.0f + 0.5f);

			for (uint x = 0; x < WIDTH; ++x)
			{
				size_t idx = ((size_t)y * WIDTH + x) * 4;
				pixels[idx + 0] = pr;
				pixels[idx + 1] = pg;
				pixels[idx + 2] = pb;
				pixels[idx + 3] = 255;
			}
		}

		auto list = device.get_upload_list();
		list->get_copy().update_texture(tex.get(), ivec3(0, 0, 0), ivec3(WIDTH, HEIGHT, 1), 0,
			reinterpret_cast<const char*>(pixels.data()), ROW_STRIDE);
		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "rainbow_vertical");
	}

	TEST(Core.HAL, LoadIconTexture)
	{
		auto& device = RenderSystem::get().device();

		auto file = FileSystem::get().get_file("resources/icon.jpg");
		ASSERT_TRUE(file != nullptr);

		auto td = HAL::texture_data::load_texture(file, 0);
		ASSERT_TRUE(td != nullptr);
		ASSERT_TRUE(!td->array.empty() && !td->array[0]->mips.empty());

		auto& mip0 = td->array[0]->mips[0];
		ASSERT_TRUE(mip0->width > 0 && mip0->height > 0);

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(td->format, {mip0->width, mip0->height}, 1, 1),
			HAL::HeapType::DEFAULT);

		auto list = device.get_upload_list();
		list->get_copy().update_texture(tex.get(), ivec3(0, 0, 0),
			ivec3(mip0->width, mip0->height, 1), 0,
			reinterpret_cast<const char*>(mip0->data.data()), mip0->width_stride);
		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "icon");
	}

	// Texture clear tests
	TEST(Core.HAL, ClearTextureToColor)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = 64;
		constexpr uint HEIGHT = 64;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"ClearTextureToColor");

		HAL::Texture2DView view(tex, *list);

		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		list->get_graphics().set_rtv(compiled,
			HAL::RTOptions::SetHandles | HAL::RTOptions::ClearColor,
			0, 0, vec4(0.2f, 0.5f, 0.8f, 1.0f));

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "clear_color");
	}

	TEST(Core.HAL, ClearIgnoresScissor)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = 64;
		constexpr uint HEIGHT = 64;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::RenderTarget),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"ClearIgnoresScissor");

		HAL::Texture2DView view(tex, *list);
		HAL::CompiledRT compiled;
		compiled.table_rtv = view.renderTarget;

		// Fill entire texture with orange as baseline
		list->get_graphics().set_rtv(compiled,
			HAL::RTOptions::SetHandles | HAL::RTOptions::ClearColor,
			0, 0, vec4(1.0f, 0.5f, 0.0f, 1.0f));

		// Restrict scissor to top-left quadrant
		list->get_graphics().set_scissor({0, 0, (long)(WIDTH / 2), (long)(HEIGHT / 2)});

		// Clear to blue — should cover the entire texture regardless of scissor
		list->get_graphics().set_rtv(compiled,
			HAL::RTOptions::ClearColor,
			0, 0, vec4(0.0f, 0.4f, 0.9f, 1.0f));

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "clear_scissor");
	}

	TEST(Core.HAL, ClearTextureUAV)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint WIDTH  = 64;
		constexpr uint HEIGHT = 64;

		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {WIDTH, HEIGHT}, 1, 1,
				HAL::ResFlags::UnorderedAccess),
			HAL::HeapType::DEFAULT);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"ClearTextureUAV");

		HAL::Texture2DView view(tex, *list);
		list->clear_uav(view.rwTexture2D, vec4(0.1f, 0.8f, 0.3f, 1.0f));

		list->execute_and_wait();

		ASSERT_TEXTURE(tex.get(), "clear_uav");
	}

	// neon.png as material textures load it (Texture::load_native): generated mips.
	HAL::texture_data::ptr load_mipped_test_texture()
	{
		auto file = FileSystem::get().get_file(L"resources/neon.png");
		if (!file)
			throw Test::TestFailure("resources/neon.png missing");
		auto data = HAL::texture_data::load_texture(file, HAL::texture_data::LoadFlags::GENERATE_MIPS);
		if (!data || data->mip_maps <= 3)
			throw Test::TestFailure("neon.png did not load with a mip chain");
		return data;
	}

	// Reads every mip of `texture` back and compares it byte for byte with `expected`.
	void expect_mips_match(HAL::TextureResource* texture, const HAL::texture_data& expected, HAL::Format format)
	{
		auto& device = RenderSystem::get().device();
		const uint mips = expected.mip_maps;
		const auto& want_mips = expected.array[0]->mips;

		std::vector<HAL::texture_data::ptr> actual(mips);
		std::vector<std::future<bool>> pending;
		auto list = device.get_upload_list();
		for (uint m = 0; m < mips; ++m)
			pending.push_back(list->get_copy().read_texture(texture, m,
				[&actual, m, w = want_mips[m]->width, h = want_mips[m]->height, format](std::span<std::byte> bytes, HAL::texture_layout layout) {
					actual[m] = HAL::texture_data::from_readback(w, h, format, bytes, layout);
				}));
		list->execute_and_wait();
		for (auto& p : pending)
			p.wait();

		for (uint m = 0; m < mips; ++m)
		{
			if (!actual[m])
				throw Test::TestFailure("readback failed for mip " + std::to_string(m));
			auto& got  = *actual[m]->array[0]->mips[0];
			auto& want = *want_mips[m];
			const uint row_bytes = got.width_stride;
			for (uint y = 0; y < want.num_rows; ++y)
				if (std::memcmp(got.data.data() + y * row_bytes, want.data.data() + y * want.width_stride, row_bytes) != 0)
					throw Test::TestFailure("mip " + std::to_string(m) + " (" + std::to_string(want.width) + "x" + std::to_string(want.height)
						+ ") differs from the loaded data at row " + std::to_string(y));
		}
	}

	// Every mip of a loaded texture must reach the GPU as loaded.
	TEST(Core.HAL, Texture_LoadedMipsUploadIntact)
	{
		auto data = load_mipped_test_texture();
		// upload_data retypes data->format; keep what the CPU mips hold.
		const HAL::Format format = data->format;
		auto expected = data;
		auto texture = HAL::Texture::create(RenderSystem::get().device(), data);
		ASSERT_TRUE(texture != nullptr);
		expect_mips_match(texture->resource.get(), *expected, format);
	}

	// The same, through the texture cache: the second get_resource deserializes
	// the cache file written by the first, the path every later app start takes.
	TEST(Core.HAL, Texture_CachedMipsIntact)
	{
		auto data = load_mipped_test_texture();
		const HAL::Format format = data->format;

		const HAL::texure_header header(L"resources/neon.png", false, true);
		ASSERT_TRUE(HAL::Texture::get_resource(header) != nullptr);
		HAL::Texture::reset_manager();
		auto cached = HAL::Texture::get_resource(header);
		ASSERT_TRUE(cached != nullptr);
		ASSERT_TRUE(cached->get_desc().as_texture().MipLevels == data->mip_maps);
		expect_mips_match(cached->resource.get(), *data, format);
	}

	// Texture filtering alone, against a D3D12 golden: every static sampler,
	// UNORM and sRGB views, footprints from magnified to past the last mip and
	// anisotropy up to 16:1 (fgt_sampler_probe.hlsl). Gradients are explicit,
	// so a mismatch here is the sampler or texture, not rasterization or
	// derivatives.
	TEST(Core.HAL, Sampler_FilteringProbe)
	{
		auto& device = RenderSystem::get().device();
		auto data = load_mipped_test_texture();
		auto texture = HAL::Texture::create(device, data);
		ASSERT_TRUE(texture != nullptr);

		constexpr uint W = 9 * 128, H = 512;
		auto output = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { W, H }, 1, 1, HAL::ResFlags::UnorderedAccess),
			HAL::HeapType::DEFAULT);

		auto list = device.get_queue(HAL::CommandListType::DIRECT)->get_free_list();
		list->begin(L"Sampler probe");
		HAL::Texture2DView output_view(output, *list);
		auto& compute = list->get_compute();
		compute.set_signature(Layouts::DefaultLayout);
		{
			Slots::FrameGraphTests::FGTSamplerProbe probe;
			probe.GetSize()     = uint2(W, H);
			probe.GetTex()      = texture->texture_2d().texture2D;
			probe.GetTex_srgb() = texture->texture_2d_srgb().texture2D;
			probe.GetOutput()   = output_view.rwTexture2D;
			compute.set(probe);
		}
		compute.set_pipeline<PSOS::FrameGraphTests::FGTSamplerProbe>();
		compute.dispatch(ivec2(W, H), ivec2(8, 8));
		list->execute_and_wait();

		// The footprint sweeps across every mip boundary, where nearest-mip
		// selection rounds by each vendor's fixed-point LOD: AMD against the
		// NVIDIA golden differs on ~140 pixels there (point picks the other
		// mip; trilinear by 3/255). NVIDIA D3D12 vs Vulkan is exact.
		ASSERT_TEXTURE_EX(output.get(), "sampler_probe", .max_mismatch_fraction = 0.0005);
	}

	// Vulkan keeps Append/Consume counters in their own binding, written apart
	// from the view: reusing a slot for a counter-less or null UAV must clear it,
	// or the slot keeps a counter descriptor that may name a freed buffer.
	TEST(Core.HAL, Descriptor_CounterSlotReuse)
	{
		if (HAL::get_backend_name() != "vulkan")
			throw TestSkipped("D3D12 UAV descriptors embed their counter");

		auto& device = RenderSystem::get().device();
		auto data    = std::make_shared<HAL::Buffer>(device, HAL::ResourceDesc::Buffer(1024, HAL::ResFlags::UnorderedAccess), HAL::HeapType::DEFAULT);
		auto counter = std::make_shared<HAL::Buffer>(device, HAL::ResourceDesc::Buffer(256, HAL::ResFlags::UnorderedAccess), HAL::HeapType::DEFAULT);
		auto heap    = std::make_shared<HAL::DescriptorHeap>(device, 4, HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible);

		auto uav = [&](std::shared_ptr<HAL::Resource> counter_res) {
			return HAL::Views::UnorderedAccess{ data, HAL::Format::UNKNOWN,
				HAL::Views::UnorderedAccess::Buffer{ 0, 256, 4, false, 0, std::move(counter_res) } };
		};
		auto bound = [](auto& h, uint slot) {
			if constexpr (requires { h.has_counter_descriptor(slot); })
				return h.has_counter_descriptor(slot);
			else
				return false;
		};

		(*heap)[0].place(uav(counter));
		ASSERT_TRUE(bound(*heap, 0));
		(*heap)[0].place(uav(nullptr));
		ASSERT_FALSE(bound(*heap, 0));

		(*heap)[1].place(uav(counter));
		ASSERT_TRUE(bound(*heap, 1));
		(*heap)[1].place(HAL::Views::UnorderedAccess{ nullptr, HAL::Format::UNKNOWN, HAL::Views::UnorderedAccess::Buffer{} });
		ASSERT_FALSE(bound(*heap, 1));
	}
}

