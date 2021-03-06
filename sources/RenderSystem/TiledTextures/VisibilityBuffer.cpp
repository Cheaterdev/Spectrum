#include "pch.h"



VisibilityBuffer::VisibilityBuffer(ivec3 sizes) :sizes(sizes)
{
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8_UINT, sizes.x, sizes.y, sizes.z, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE);

	buffer = std::make_shared<Render::Texture>(desc);
	buffer->set_name("VisibilityBuffer::buffer");

	load_tiles_buffer = std::make_shared<Render::StructuredBuffer<uint4>>(sizes.x * sizes.y * sizes.z, counterType::HELP_BUFFER, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}
/*
void VisibilityBuffer::wait_for_results()
{
	if (waiter.valid())
		waiter.get();
}
*/


std::future<visibility_update> VisibilityBuffer::update(CommandList::ptr& list)
{


	auto promise = std::make_shared<std::promise<visibility_update>>();
	auto future = promise->get_future();
	auto& compute = list->get_compute();
	auto& copy = list->get_copy();

	struct _info
	{
		UINT size;
	};

	list->clear_counter(load_tiles_buffer);

	{
		Slots::VoxelVisibility data;

		data.GetVisibility()= buffer->create_view<Render::TextureView>(*list->frame_resources).texture3D;
		data.GetVisible_tiles() = load_tiles_buffer->appendStructuredBuffer;
		data.set(compute);
	}

	compute.set_pipeline(GetPSO<PSOS::VoxelVisibility>());
	compute.dispach(sizes);


	auto info = std::make_shared<_info>();
	copy.read_buffer(load_tiles_buffer->help_buffer.get(), 0, 4, [this, info](const char* data, UINT64 size)
		{
			info->size = *reinterpret_cast<const UINT*>(data);
		});


	  copy.read_buffer(load_tiles_buffer.get(), 0, load_tiles_buffer->get_size(), [this, info, promise](const char* data, UINT64 size)
		{
			PROFILE(L"Read Tiles");
			const uint4* tiles = reinterpret_cast<const uint4*>(data);

			visibility_update update;

			for (int i = 0; i < info->size; i++)
				update.tiles_to_load.emplace_back(tiles[i]);

			promise->set_value(update);
		});


	list->clear_uav(buffer->texture_3d()->get_static_uav());

	return future;
}

void VisibilityBufferUniversal::process_tile_readback(ivec3 pos, char level)
{
	PROFILE(L"process_tile_readback");
	on_process(pos, level);
}
