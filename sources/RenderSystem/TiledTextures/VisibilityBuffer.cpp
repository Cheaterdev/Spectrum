
module Graphics:VisibilityBuffer;
import <RenderSystem.h>;

import HAL;

VisibilityBuffer::VisibilityBuffer(uint3 sizes) :sizes(sizes)
{
	auto desc = HAL::ResourceDesc::Tex3D(HAL::Format::R8_UINT, { sizes.x, sizes.y, sizes.z }, 1, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess/*, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE*/);

	buffer = std::make_shared<HAL::Texture>(desc);
	buffer->resource->set_name("VisibilityBuffer::buffer");

	load_tiles_buffer = std::make_shared<HAL::StructureBuffer<uint4>>(sizes.x * sizes.y * sizes.z, HAL::counterType::HELP_BUFFER, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
}
/*
void VisibilityBuffer::wait_for_results()
{
	if (waiter.valid())
		waiter.get();
}
*/


std::future<visibility_update> VisibilityBuffer::update(HAL::CommandList::ptr& list)
{


	auto promise = std::make_shared<std::promise<visibility_update>>();
	auto future = promise->get_future();
	auto& compute = list->get_compute();
	auto& copy = list->get_copy();

	struct _info
	{
		UINT size;
	};

	load_tiles_buffer->clear_counter(list);

	{
		Slots::VoxelVisibility data;

		data.GetVisibility() = buffer->resource->create_view<HAL::Texture3DView>(*list).texture3D;
		data.GetVisible_tiles() = load_tiles_buffer->appendStructuredBuffer;
		compute.set(data);
	}

	compute.set_pipeline<PSOS::VoxelVisibility>();
	compute.dispatch(ivec3(sizes));


	auto info = std::make_shared<_info>();
	copy.read_buffer(load_tiles_buffer->help_buffer->resource.get(), 0, 4, [this, info](std::span<std::byte> memory)
		{
			info->size = *reinterpret_cast<const UINT*>(memory.data());
		});


	copy.read_buffer(load_tiles_buffer->resource.get(), 0, load_tiles_buffer->resource->get_size(), [this, info, promise](std::span<std::byte> memory)
		{
			PROFILE(L"Read Tiles");
			const uint4* tiles = reinterpret_cast<const uint4*>(memory.data());

			visibility_update update;

			for (UINT i = 0; i < info->size; i++)
				update.tiles_to_load.emplace_back(tiles[i].xyz);

			promise->set_value(update);
		});


	list->clear_uav(buffer->texture_3d().mips[0].rwTexture3D);

	return future;
}

void VisibilityBufferUniversal::process_tile_readback(uint3 pos, char level)
{
	PROFILE(L"process_tile_readback");
	on_process(pos, level);
}
