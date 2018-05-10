#include "pch.h"




bool Texture3DTiled::is_good(ivec3 pos, int mip)
{
	if (mip < 0) return false;

	if (mip >= mips.size()) return false;

	if (pos.x < 0) return false;

	if (pos.y < 0) return false;

	if (pos.z < 0) return false;

	if (pos.x >= mips[mip].tiles.x) return false;

	if (pos.y >= mips[mip].tiles.y) return false;

	if (pos.z >= mips[mip].tiles.z) return false;

	return true;
}

Tile::ptr Texture3DTiled::create_tile_internal(ivec3 position, int mip_level)
{
	if (!is_good(position, mip_level))
		return nullptr;

	auto& tile = get_tile(mip_level, position);


	if (tile) {
		if (!tile->page)
			if (on_tile_load) on_tile_load(tile);

		return tile;
	}
	{
		std::lock_guard<std::mutex> g(m);

		if (tile) return tile;


		Tile::ptr new_tile(new Tile);
		new_tile->position = position;
		new_tile->mip_level = mip_level;
		new_tile->subresource = D3D12CalcSubresource(mip_level, 0, 0, texture->get_desc().MipLevels, texture->get_desc().DepthOrArraySize);

		//		mips[mip_level].load_queue.emplace_back(new_tile);
		tile = new_tile;

		if (exists_parent(tile))
		{
			auto &parent = get_parent_tile(tile);
			if (parent)	parent->childs_count++;
		}
		tile->childs_count = get_child_count(tile);


	}
	if (on_tile_load) on_tile_load(tile);

	return tile;
}

ivec3 Texture3DTiled::get_tiles_count(int mip_level /*= 0*/)
{
	return mips[mip_level].tiles;
}

ivec3 Texture3DTiled::get_voxels_per_tile()
{
	return one_tile_size;
}

int Texture3DTiled::get_used_tiles()
{
	return	heap_manager_static.get_used_tiles();
}

Tile::ptr& Texture3DTiled::get_tile(int mip_level, ivec3 pos)
{
	return mips[mip_level].tile_positions[pos.x + pos.y * mips[mip_level].tiles.x + pos.z * mips[mip_level].tiles.x * mips[mip_level].tiles.y];
}

bool Texture3DTiled::exists(ivec3 pos, int mip_level)
{
	if (!is_good(pos, mip_level)) return 0;

	return !!get_tile(mip_level, pos);
}

bool Texture3DTiled::exists_parent(Tile::ptr& tile)
{
	return exists(tile->position / 2, tile->mip_level + 1);
}

Tile::ptr Texture3DTiled::get_tile_safe(int mip_level, ivec3 pos)
{
	if (!is_good(pos, mip_level)) return nullptr;

	return mips[mip_level].tile_positions[pos.x + pos.y * mips[mip_level].tiles.x + pos.z * mips[mip_level].tiles.x * mips[mip_level].tiles.y];
}

uint32_t Texture3DTiled::get_max_tiles_count(int mip /*= 0*/)
{
	return mips[mip].tiles.x*mips[mip].tiles.y*mips[mip].tiles.z;
}

int Texture3DTiled::get_index(ivec3 position, int mip)
{
	return position.x + position.y* mips[mip].tiles.x + position.z* mips[mip].tiles.x* mips[mip].tiles.y;
}

int Texture3DTiled::get_child_count(Tile::ptr& tile)
{
	return !!exists(tile->position * 2, tile->mip_level - 1) +
		!!exists(tile->position * 2 + ivec3(1, 0, 0), tile->mip_level - 1) +
		!!exists(tile->position * 2 + ivec3(0, 1, 0), tile->mip_level - 1) +
		!!exists(tile->position * 2 + ivec3(1, 1, 0), tile->mip_level - 1);
}

Tile::ptr Texture3DTiled::create_tile(ivec3 position, int mip_level)
{
	Tile::ptr tile = create_tile_internal(position, mip_level);
	if (!tile) return nullptr;


	if (!tile->page)
	{
		heap_manager_static.place(tile);
	}
	return tile;
}

Tile::ptr& Texture3DTiled::get_parent_tile(Tile::ptr &tile)
{
	return get_tile(tile->mip_level + 1, tile->position / 2);
}

void Texture3DTiled::remove_tile(Tile::ptr &tile)
{
	if (!tile) return;

	heap_manager_static.remove(tile, true);

	if (exists_parent(tile))
	{
		auto &parent = get_parent_tile(tile);
		parent->childs_count--;
		if (parent->childs_count == 0)
			remove_tile(parent);
	}
	tile.reset();
}

void Texture3DTiled::place_tile(Tile::ptr &tile)
{
	std::lock_guard<std::mutex> g(place_mutex);
	heap_manager_static.place(tile);
	tile->state = TileState::LOADED;
}

void Texture3DTiled::remove_all()
{
	heap_manager_static.remove_all();
	for (auto&m : mips)
	{
		for (auto&e : m.tile_positions)
			e = nullptr;
	}
}

Texture3DTiled::Texture3DTiled(CD3DX12_RESOURCE_DESC desc)
{
	ComPtr<ID3D12Resource> m_Resource;
	Render::Device::get().get_native_device()->CreateReservedResource(&desc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_Resource));
	texture.reset(new Render::Texture(m_Resource));
	texture->assume_gpu_state(ResourceState::PIXEL_SHADER_RESOURCE);
	UINT num_tiles = 1;
	D3D12_PACKED_MIP_INFO mip_info;
	D3D12_TILE_SHAPE tile_shape;
	UINT num_sub_res = 20;
	UINT first_sub_res;
	D3D12_SUBRESOURCE_TILING tilings[20];
	Render::Device::get().get_native_device()->GetResourceTiling(m_Resource.Get(), &num_tiles, &mip_info, &tile_shape, &num_sub_res, 0, tilings);

	num_sub_res = mip_info.NumStandardMips;
	one_tile_size = { tile_shape.WidthInTexels, tile_shape.HeightInTexels, tile_shape.DepthInTexels };

	for (int i = 0; i < num_sub_res; i++)
	{
		mips.emplace_back(ivec3(tilings[i].WidthInTiles, tilings[i].HeightInTiles, tilings[i].DepthInTiles), tilings[i].StartTileIndexInOverallResource);
		mips.back().rows_per_tile = desc.Height / pow(2, i) / mips[i].tiles.y;
		mips.back().stride_per_tile = BitsPerPixel(desc.Format)*desc.Width / pow(2, i) / mips[i].tiles.x;
	}
	texture->set_name("Texture3DTiled::texture");

	heap_manager_static.res = texture.get();
}

void Texture3DTiled::clear(Render::CommandList& list)
{
	heap_manager_static.clear(list);
}

VisibilityBufferUniversal::ptr Texture3DTiled::create_visibility()
{
	return std::make_shared<VisibilityBufferUniversal>(mips[0].tiles);
}

void Texture3DTiled::load_all()
{
	/*for (int i = 0; i<mips[0].tiles.x; i += 1)
	for (int j = 0; j<mips[0].tiles.y; j += 1)
	for (int k = 0; k < mips[0].tiles.z; k += 1)
	{
	create_tile({ i,j,k }, 0);
	}*/
	Tile::ptr tile;

	tile.reset(new Tile);
	tile->position = { 0,0,0 };
	tile->mip_level = 5;
	tile->subresource = 5;
	//if (on_tile_load) on_tile_load(tile);

	heap_manager_static.place(tile);
}

void Texture3DTiled::update()
{
	heap_manager_static.flush_tilings(texture.get());
}

Texture3DTiledDynamic::Texture3DTiledDynamic(CD3DX12_RESOURCE_DESC desc) :Texture3DTiled(desc)
{

}

int Texture3DTiledDynamic::get_used_tiles()
{
	return	heap_manager_static.get_used_tiles() + heap_manager_dynamic.get_used_tiles();
}

void Texture3DTiledDynamic::remove_all()
{
	Texture3DTiled::remove_all();
	heap_manager_dynamic.remove_all();
}

void Texture3DTiledDynamic::place_tile(Tile::ptr &tile)
{
	std::lock_guard<std::mutex> g(place_mutex);

	if (tile->has_dynamic)
		heap_manager_dynamic.place(tile);
	else
		heap_manager_static.place(tile);

	tile->state = TileState::LOADED;
}

void Texture3DTiledDynamic::clear_static(Render::CommandList& list)
{
	heap_manager_static.clear(list);
}

void Texture3DTiledDynamic::clear_dynamic(Render::CommandList& list)
{
	heap_manager_dynamic.clear(list);
}

void Texture3DTiledDynamic::update()
{
	Texture3DTiled::update();
	heap_manager_dynamic.flush_tilings(texture.get());
}

Tile::ptr Texture3DTiledDynamic::create_dynamic_tile(ivec3 position, int mip_level)
{
	Tile::ptr tile = create_tile_internal(position, mip_level);
	if (!tile) return nullptr;


	if (!tile->has_dynamic) {

		if (tile->has_static)
		{
			tile->page_backup = tile->page;
			assert(tile->page_backup);
			tile->tile_offset_backup = tile->tile_offset;

		}

		tile->has_dynamic = true;
		heap_manager_dynamic.place(tile);
	}

	return tile;
}

bool Texture3DTiledDynamic::remove_dynamic_tile(ivec3 pos, int level)
{
	auto & tile = get_tile(level, pos);

	if (!tile) return false;

	assert(tile->has_dynamic);

	heap_manager_dynamic.remove(tile, !tile->has_static);

	if (tile->has_static)
	{
		//heap_manager_static.place(tile);
		tile->has_dynamic = false;
		assert(tile->page_backup);
		tile->page = tile->page_backup;
		tile->tile_offset = tile->tile_offset_backup;

		tile->page->place_at(tile, tile->tile_offset);
		return false;
	}

	remove_tile(tile);
	return true;
}

Tile::ptr Texture3DTiledDynamic::create_static_tile(ivec3 position, int mip_level)
{
	Tile::ptr tile = create_tile_internal(position, mip_level);

	if (!tile) return nullptr;

	tile->has_static = true;
	if (tile->has_dynamic)
	{
	//	assert(tile->page_backup);
	}


	if (!tile->page)
	{
		heap_manager_static.place(tile);
	}


	return tile;
}

bool Texture3DTiledDynamic::remove_static_tile(ivec3 pos, int level)
{
	auto & tile = get_tile(level, pos);
	if (!tile) return false;

	assert(tile->has_static);

	tile->has_static = false;
	assert(!tile->page_backup);
	if (tile->has_dynamic)
	{
		assert(!tile->page_backup);
		return false;
	}

	remove_tile(tile);
	return true;
}

void Texture3DTiledDynamic::remove_tile(Tile::ptr &tile)
{
	if (!tile) return;


	assert(!tile->page_backup);
	if (tile->has_dynamic)
		heap_manager_dynamic.remove(tile, true);
	else
		heap_manager_static.remove(tile, true);
	tile->has_dynamic = false;
	tile->has_static = false;
	if (exists_parent(tile))
	{
		auto &parent = get_parent_tile(tile);
		parent->childs_count--;
		if (parent->childs_count == 0)
			remove_tile(parent);
	}
	if (on_tile_remove)on_tile_remove(tile);
	tile.reset();
}

void Texture3DTiledDynamic::test()
{
	for (int i = 0; i<mips[0].tiles.x; i += 1)
		for (int j = 0; j<mips[0].tiles.y; j += 1)
			for (int k = 0; k < mips[0].tiles.z; k += 1)
			{
				create_static_tile({ i,j,k }, 0);
			}
}