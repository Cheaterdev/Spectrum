export module Test.HAL;

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;

SETUP_CATEGORY(Core.HAL, []() {
	auto device = HAL::Device::create_singleton();
	if (!device)
		Test::TestRegistry::Instance().SkipCategory("Core.HAL", "no suitable GPU device found");
});

TEARDOWN_CATEGORY(Core.HAL, []() {
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
}

