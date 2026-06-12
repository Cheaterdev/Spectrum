export module Test.HAL;

#define TEST_MODULE_ID HAL

export import Test.Framework;
export import Test.HAL.TextureUtils;
import Test.HAL.Rendering;
import Test.HAL.SIG;
import Test.HAL.UI;
import Test.GUI;
import Test.FrameGraph;

import Core;
import HAL;
import Graphics;
import GUI;
import TextSystem;
SETUP_CATEGORY(Core.HAL, []() {
	auto device = HAL::Device::create_singleton();
	if (!device)
		Test::TestRegistry::Instance().SkipCategory("Core.HAL", "no suitable GPU device found");
	else
		AssetManager::create();
});

TEARDOWN_CATEGORY(Core.HAL, []() {
	Skin::reset();
	Fonts::FontSystem::reset();
	AssetManager::reset();
	HAL::Device::reset();
});

export namespace Test
{
	// Device tests
	TEST(Core.HAL, DeviceCreation)
	{
		ASSERT_TRUE(&HAL::Device::get() != nullptr);
	}

	TEST(Core.HAL, DeviceProperties)
	{
		const auto& props = HAL::Device::get().get_properties();
		ASSERT_TRUE(!props.name.empty());
	}

	TEST(Core.HAL, DeviceVRAM)
	{
		size_t vram = HAL::Device::get().get_vram();
		ASSERT_TRUE(vram > 0);
	}

	// Queue tests
	TEST(Core.HAL, GetDirectQueue)
	{
		auto& queue = HAL::Device::get().get_queue(HAL::CommandListType::DIRECT);
		ASSERT_TRUE(queue != nullptr);
	}

	TEST(Core.HAL, GetComputeQueue)
	{
		auto& queue = HAL::Device::get().get_queue(HAL::CommandListType::COMPUTE);
		ASSERT_TRUE(queue != nullptr);
	}

	TEST(Core.HAL, GetCopyQueue)
	{
		auto& queue = HAL::Device::get().get_queue(HAL::CommandListType::COPY);
		ASSERT_TRUE(queue != nullptr);
	}

	TEST(Core.HAL, QueueSignalAndWait)
	{
		auto& queue = HAL::Device::get().get_queue(HAL::CommandListType::DIRECT);
		queue->signal_and_wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, QueueSignal)
	{
		auto& queue = HAL::Device::get().get_queue(HAL::CommandListType::DIRECT);
		auto waiter = queue->signal();
		waiter.wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, ComputeQueueSignalAndWait)
	{
		auto& queue = HAL::Device::get().get_queue(HAL::CommandListType::COMPUTE);
		queue->signal_and_wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, CopyQueueSignalAndWait)
	{
		auto& queue = HAL::Device::get().get_queue(HAL::CommandListType::COPY);
		queue->signal_and_wait();
		ASSERT_TRUE(true);
	}

	// Buffer creation tests
	TEST(Core.HAL, CreateUploadBuffer)
	{
		auto& device = HAL::Device::get();
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256, HAL::ResFlags::None),
			HAL::HeapType::UPLOAD);
		ASSERT_TRUE(buffer != nullptr);
	}

	TEST(Core.HAL, CreateDefaultBuffer)
	{
		auto& device = HAL::Device::get();
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(buffer != nullptr);
	}

	TEST(Core.HAL, CreateReadbackBuffer)
	{
		auto& device = HAL::Device::get();
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(256, HAL::ResFlags::None),
			HAL::HeapType::READBACK);
		ASSERT_TRUE(buffer != nullptr);
	}

	TEST(Core.HAL, CreateLargeBuffer)
	{
		auto& device = HAL::Device::get();
		constexpr size_t SIZE = 1024 * 1024; // 1 MB
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(buffer != nullptr);
		ASSERT_TRUE(buffer->get_size() >= SIZE);
	}

	TEST(Core.HAL, UploadBufferCPUAccess)
	{
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
		constexpr size_t SIZE = 256;
		auto buffer = std::make_shared<HAL::Buffer>(device,
			HAL::ResourceDesc::Buffer(SIZE, HAL::ResFlags::None),
			HAL::HeapType::READBACK);
		auto data = buffer->cpu_data();
		ASSERT_TRUE(data.data() != nullptr);
	}

	TEST(Core.HAL, BufferHeapType)
	{
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {64, 64}, 1, 1),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(tex != nullptr);
	}

	TEST(Core.HAL, CreateTexture2DUAV)
	{
		auto& device = HAL::Device::get();
		auto tex = std::make_shared<HAL::TextureResource>(device,
			HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, {64, 64}, 1, 1,
				HAL::ResFlags::UnorderedAccess | HAL::ResFlags::ShaderResource),
			HAL::HeapType::DEFAULT);
		ASSERT_TRUE(tex != nullptr);
	}

	TEST(Core.HAL, TextureResourceDesc)
	{
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"EmptyDirect");
		list->execute_and_wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, ExecuteEmptyCopyCommandList)
	{
		auto& device = HAL::Device::get();
		auto& queue = device.get_queue(HAL::CommandListType::COPY);
		auto list = queue->get_free_list();
		list->begin(L"EmptyCopy");
		list->execute_and_wait();
		ASSERT_TRUE(true);
	}

	TEST(Core.HAL, ExecuteMultipleCommandLists)
	{
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();

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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
		auto& device = HAL::Device::get();
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
}

