module Graphics:GPUTilesBuffer;


void GPUTilesBuffer::set_size(ivec3 size, ivec3 shape)
{
	this->shape = shape;

	tile_positions.resize(size, -1);
	buffer = HAL::StructuredBufferView<ivec3>(size.x * size.y * size.z);
	dispatch_buffer = HAL::StructuredBufferView<DispatchArguments>(1, HAL::counterType::NONE, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
}

void GPUTilesBuffer::clear()
{
	tiles_updated = true;
	used_tiles.clear();
	tile_positions.fill(-1);
}

uint32_t GPUTilesBuffer::size()
{
	return (uint32_t)used_tiles.size();
}

void GPUTilesBuffer::insert(ivec3 pos)
{
	int tile_pos = tile_positions[pos];
	if (tile_pos == -1)
	{
		used_tiles.emplace_back(pos);
		tiles_updated = true;
		tile_positions[pos] = static_cast<int>(used_tiles.size() - 1);
	}
}

void GPUTilesBuffer::erase(ivec3 pos)
{
	int tile_pos = tile_positions[pos];
	if (tile_pos != -1)
	{
		used_tiles[tile_pos] = used_tiles.back();
		tile_positions[used_tiles[tile_pos]] = tile_pos;
		used_tiles.resize(used_tiles.size() - 1);
		tile_positions[pos] = -1;
		tiles_updated = true;
	}
}

void GPUTilesBuffer::update(HAL::CommandList::ptr list)
{
	if (tiles_updated&&!used_tiles.empty()) {
		list->get_copy().update(buffer, 0, used_tiles);
		DispatchArguments args;

		args.ThreadGroupCountX = static_cast<UINT>(used_tiles.size() * shape.x / 4);
		args.ThreadGroupCountY = shape.y / 4;
		args.ThreadGroupCountZ = shape.z / 4;

		list->get_copy().update(dispatch_buffer, 0, std::span{static_cast<D3D12_DISPATCH_ARGUMENTS*>(&args),1});
	}
	tiles_updated = false;
}
