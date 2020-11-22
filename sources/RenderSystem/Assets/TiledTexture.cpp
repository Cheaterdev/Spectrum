#include "pch.h"

unsigned HeapPage::allocate()
{
    if (free_tiles.size())
    {
        int offset = free_tiles.front();
        free_tiles.pop_front();
        return offset;
    }

    else
    {
        if (heap_count < max_count)
            return heap_count++;
        else
            return INVALID_VALUE;
    }
}

HeapPage::HeapPage()
{
    max_count = 32;
    D3D12_HEAP_DESC h;
    h.Alignment = 0;
	h.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;// D3D12_HEAP_FLAG_NONE;
    h.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    h.Properties.CreationNodeMask = 0;
    h.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    h.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    h.Properties.VisibleNodeMask = 0;
    h.SizeInBytes = (64 * 1024) * max_count;
    Render::Device::get().get_native_device()->CreateHeap(&h, IID_PPV_ARGS(&tile_heap));

	ComPtr<ID3D12Resource> m_Resource;

	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8_UINT,min(h.SizeInBytes, UINT64(16384u)),(UINT)(Math::AlignUp(h.SizeInBytes, 16384)/ 16384),1,1,1,0,D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	Render::Device::get().get_native_device()->CreatePlacedResource(tile_heap.Get(), 0, &desc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_Resource));
	data.reset(new Render::Texture(m_Resource, Render::ResourceState::NON_PIXEL_SHADER_RESOURCE));

	for (unsigned int i = 0; i<max_count; i++)
	{
		ComPtr<ID3D12Resource> m_Resource;

		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8_UINT, 256, 256, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		Render::Device::get().get_native_device()->CreatePlacedResource(tile_heap.Get(), i * 65536, &desc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_Resource));
		tiles.emplace_back(new Render::Resource(m_Resource, ResourceState::NON_PIXEL_SHADER_RESOURCE, true));
	}
}

bool HeapPage::place(Tile::ptr& tile)
{
    unsigned int offset = allocate();

    if (offset == INVALID_VALUE)
        return false;

    D3D12_TILED_RESOURCE_COORDINATE TRC;
    TRC.X = tile->position.x;
    TRC.Y = tile->position.y;
    TRC.Z = tile->position.z;
    TRC.Subresource = tile->subresource;
    D3D12_TILE_REGION_SIZE TRS;
    TRS.UseBox = TRUE;
    TRS.Width = 1;
    TRS.Height = 1;
    TRS.Depth = 1;
    TRS.NumTiles = TRS.Width * TRS.Height * TRS.Depth;
    startCoordinates.push_back(TRC);
    regionSizes.push_back(TRS);
    rangeFlags.push_back(D3D12_TILE_RANGE_FLAG_REUSE_SINGLE_TILE);
    heapRangeStartOffsets.push_back(offset);
    rangeTileCounts.push_back(TRS.NumTiles);
    tile->tile_offset = offset;
    tile->page = this;
    return true;
}

void HeapPage::remove(Tile::ptr& tile)
{
	//place_region(tile->position, {1,1,1},0,0);
	free_tiles.push_back(tile->tile_offset);

	tile->tile_offset = -1;
	tile->page = nullptr;
}

void HeapPage::place_at(Tile::ptr& tile, unsigned int offset)
{
    D3D12_TILED_RESOURCE_COORDINATE TRC;
    TRC.X = tile->position.x;
    TRC.Y = tile->position.y;
    TRC.Z = tile->position.z;
    TRC.Subresource = tile->subresource;
    D3D12_TILE_REGION_SIZE TRS;
    TRS.UseBox = TRUE;
    TRS.Width = 1;
    TRS.Height = 1;
    TRS.Depth = 1;
    TRS.NumTiles = TRS.Width * TRS.Height * TRS.Depth;
    startCoordinates.push_back(TRC);
    regionSizes.push_back(TRS);
    rangeFlags.push_back(D3D12_TILE_RANGE_FLAG_REUSE_SINGLE_TILE);
    heapRangeStartOffsets.push_back(offset);
    rangeTileCounts.push_back(TRS.NumTiles);
}


void HeapPage::zero_tile(Tile::ptr& tile)
{
	D3D12_TILED_RESOURCE_COORDINATE TRC;
	TRC.X = tile->position.x;
	TRC.Y = tile->position.y;
	TRC.Z = tile->position.z;
	TRC.Subresource = tile->subresource;
	D3D12_TILE_REGION_SIZE TRS;
	TRS.UseBox = TRUE;
	TRS.Width = 1;
	TRS.Height = 1;
	TRS.Depth = 1;
	TRS.NumTiles = TRS.Width * TRS.Height * TRS.Depth;
	startCoordinates.push_back(TRC);
	regionSizes.push_back(TRS);
	rangeFlags.push_back(D3D12_TILE_RANGE_FLAG_NULL);
	heapRangeStartOffsets.push_back(0);
	rangeTileCounts.push_back(TRS.NumTiles);
}
void HeapPage::flush_tilings(Render::Resource* res)
{
    if (startCoordinates.size())
        Render::Device::get().get_queue(Render::CommandListType::DIRECT)->update_tile_mappings(
            res->get_native().Get(),
            UINT(startCoordinates.size()),
            &startCoordinates[0],
            &regionSizes[0],
            tile_heap.Get(),
			UINT(startCoordinates.size()),
            &rangeFlags[0],
            &heapRangeStartOffsets[0],
            &rangeTileCounts[0],
            D3D12_TILE_MAPPING_FLAG_NONE
        );

    startCoordinates.clear();
    regionSizes.clear();
    rangeFlags.clear();
    heapRangeStartOffsets.clear();
    rangeTileCounts.clear();
}

void HeapPage::place_region(ivec3 pos, ivec3 size, unsigned int subresource, unsigned int offset)
{
    D3D12_TILED_RESOURCE_COORDINATE TRC;
    TRC.X = pos.x;
    TRC.Y = pos.y;
    TRC.Z = pos.z;
    TRC.Subresource = subresource;
    D3D12_TILE_REGION_SIZE TRS;
    TRS.UseBox = TRUE;
    TRS.Width = size.x;
    TRS.Height = size.y;
    TRS.Depth = size.z;
    TRS.NumTiles = TRS.Width * TRS.Height * TRS.Depth;
    startCoordinates.push_back(TRC);
    regionSizes.push_back(TRS);
    rangeFlags.push_back(D3D12_TILE_RANGE_FLAG_REUSE_SINGLE_TILE);
    heapRangeStartOffsets.push_back(offset);
    rangeTileCounts.push_back(TRS.NumTiles);
}

Tile::ptr& TiledTexture::get_tile(int mip_level, ivec3 pos)
{
    return mips[mip_level].tile_positions[pos.x + pos.y * mips[mip_level].tiles.x + pos.z * mips[mip_level].tiles.x * mips[mip_level].tiles.y];
}

void TiledTexture::clear_tilings()
{
    heap_manager.clear_all(mips[0].tiles, (UINT)(mips.size()));
}

void TiledTexture::make_tile_visible(int mip_level, ivec3 pos)
{
    if (mip_level >= mips.size()) return;

    auto& tile = get_tile(mip_level, pos);

    if (tile && tile->last_visible < step)
    {
        tile->last_visible = step;
        make_tile_visible(mip_level + 1, pos / 2);
    }
}

void TiledTexture::zero_tile(Tile::ptr& tile)
{
    tile->state = TileState::FREED;
    heap_manager.remove(tile);
    UINT pixels = 1<<tile->mip_level;
    ivec3 mip_position = tile->position * pixels;
	UINT mip_index = mip_position.x + mip_position.y * residency_data->width + mip_position.z * residency_data->width * residency_data->height;

    for (UINT i = 0; i < pixels; i++)
        for (UINT j = 0; j < pixels; j++)
        {
            auto position = mip_index + i + j *  residency_data->width;
            residency_data->array[0]->mips[0]->data[position * 4] = std::max((int)(unsigned char)residency_data->array[0]->mips[0]->data[position * 4], tile->mip_level + 1);
            residency_data->array[0]->mips[0]->data[position * 4 + 1] = residency_data->array[0]->mips[0]->data[position * 4];
            residency_data->array[0]->mips[0]->data[position * 4 + 2] = residency_data->array[0]->mips[0]->data[position * 4];
            residency_data->array[0]->mips[0]->data[position * 4 + 3] = 255;
        }

    residency_changed = true;
    tile.reset();
}

void TiledTexture::zero_tile(int mip_level, ivec3 position)
{
    if (!is_good(position, mip_level)) return;

    auto& tile = get_tile(mip_level, position);

    if (tile)
    {
        zero_tile(tile);
        zero_tile(mip_level - 1, position * 2);
        zero_tile(mip_level - 1, position * 2 + ivec3(1, 0, 0));
        zero_tile(mip_level - 1, position * 2 + ivec3(0, 1, 0));
        zero_tile(mip_level - 1, position * 2 + ivec3(1, 1, 0));
    }
}

void TiledTexture::add_tile(Tile::ptr tile)
{
    tile->subresource = D3D12CalcSubresource(tile->mip_level, 0, 0, tiled_tex->get_desc().MipLevels, tiled_tex->get_desc().DepthOrArraySize);
    heap_manager.place(tile);
    tile->state = TileState::LOADED;
}

bool TiledTexture::is_good(ivec3 pos, int mip)
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

Asset_Type TiledTexture::get_type()
{
    return Asset_Type::TILED_TEXTURE;
}

TiledTexture::TiledTexture()
{
}

TiledTexture::TiledTexture(std::string file_name)
{
    this->file_name = file_name;
    tiles_file_name = file_name + ".tiles";
    additional_files.insert(convert(tiles_file_name));
    //	auto tex_data = texture_data::load_texture(FileSystem::get().get_file(file_name), false, true);
    //   init();
    //    return;
    auto tex_data = texture_data::load_texture(FileSystem::get().get_file(file_name), texture_data::LoadFlags::GENERATE_MIPS | texture_data::LoadFlags::COMPRESS);
    ComPtr<ID3D12Resource> m_Resource;
    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(tex_data->format, tex_data->width, tex_data->height, 1, tex_data->mip_maps - 7, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE);
    format = tex_data->format;
    format_size = (UINT)(DirectX::BitsPerPixel(tex_data->format) / 4);
    Render::Device::get().get_native_device()->CreateReservedResource(&desc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_Resource));
    tiled_tex.reset(new Render::Texture(m_Resource, ResourceState::PIXEL_SHADER_RESOURCE));
    UINT num_tiles = 1;
    D3D12_PACKED_MIP_INFO mip_info;
    D3D12_TILE_SHAPE tile_shape;
    UINT num_sub_res = 20;
  //  UINT first_sub_res;
    D3D12_SUBRESOURCE_TILING tilings[20];
    Render::Device::get().get_native_device()->GetResourceTiling(m_Resource.Get(), &num_tiles, &mip_info, &tile_shape, &num_sub_res, 0, tilings);
    num_sub_res = mip_info.NumStandardMips;

    for (UINT i = 0; i < num_sub_res; i++)
    {
        mips.emplace_back(ivec3(tilings[i].WidthInTiles, tilings[i].HeightInTiles, tilings[i].DepthInTiles), tilings[i].StartTileIndexInOverallResource);
        mips.back().rows_per_tile = tex_data->array[0]->mips[i]->num_rows / mips[i].tiles.y;
        mips.back().stride_per_tile = tex_data->array[0]->mips[i]->width_stride / mips[i].tiles.x;
    }

    sizes = { tex_data->width, tex_data->height, 1 };
    one_tile_size = { tile_shape.WidthInTexels, tile_shape.HeightInTexels, tile_shape.DepthInTexels };
    //	one_tile_size_gpu = one_tile_size;
    //	one_tile_size_gpu.y= one_tile_size_gpu.y*sizes.y/
    auto archive = ZipArchive::Create();
    int index = 0;

    for (int mip = 0; mip < mips.size(); mip++)
        for (int j = 0; j < mips[mip].tiles.y; j++)
            for (int i = 0; i < mips[mip].tiles.x; i++)
            {
                //      int rows_per_tile = tex_data->array[0]->mips[0].num_rows / mips[mip].tiles.y;
                //      int stride_per_tile = tex_data->array[0]->mips[0].width_stride / mips[mip].tiles.x;
                size_t a, row_bytes, num_rows;
                GetSurfaceInfo2(one_tile_size.x, one_tile_size.y, tex_data->format, &a, &row_bytes, &num_rows);
                std::string data;
                data.resize(a * one_tile_size.z);
                //       data.resize(one_tile_size.x * one_tile_size.y * one_tile_size.z * format_size);
                auto width = tex_data->array[0]->mips[mip]->width_stride;
                size_t data_i = j * width * num_rows + i *  mips[mip].stride_per_tile; //(i + mips[mip].tiles.x * j) * one_tile_size.x * format_size;
                ivec3 cropped_rect = one_tile_size;//ivec3::min(ivec3(tex_data->array[0]->mips[mip].width - i * one_tile_size.x, tex_data->array[0]->mips[mip].height - j * one_tile_size.y, 1), one_tile_size);

                for (size_t y = 0; y < num_rows; y++)
                    memcpy(const_cast<char*>(data.data()) + y * row_bytes, &tex_data->array[0]->mips[mip]->data[data_i + y * width], row_bytes);

                DataPacker::create_entry(archive, std::to_string(index), data, true);
                index++;
            }

    FileSystem::get().save_data(convert(tiles_file_name), DataPacker::zip_to_string(archive));
    init();
}

void TiledTexture::init()
{
    if (!tiled_tex)
    {
        ComPtr<ID3D12Resource> m_Resource;
        CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(format, sizes.x, sizes.y, sizes.z, (UINT16)mips.size(), 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE);
        Render::Device::get().get_native_device()->CreateReservedResource(&desc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr, IID_PPV_ARGS(&m_Resource));
        tiled_tex.reset(new Render::Texture(m_Resource, ResourceState::PIXEL_SHADER_RESOURCE));
    }

    clear_tilings();
    clear_data.resize(mips[0].tiles.x * mips[0].tiles.y * mips[0].tiles.z, 255);
   
    {
        residency_data.reset(new texture_data(1, 1, mips[0].tiles.x, mips[0].tiles.y, 1, DXGI_FORMAT_R8G8B8A8_UNORM));
        memset(residency_data->array[0]->mips[0]->data.data(), 255, residency_data->array[0]->mips[0]->data.size());
        residency_data_uploaded.reset(new texture_data(1, 1, mips[0].tiles.x, mips[0].tiles.y, 1, DXGI_FORMAT_R8G8B8A8_UNORM));
        memset(residency_data_uploaded->array[0]->mips[0]->data.data(), 255, residency_data_uploaded->array[0]->mips[0]->data.size());
        CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, mips[0].tiles.x, mips[0].tiles.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE, D3D12_TEXTURE_LAYOUT_UNKNOWN);
        residency_texture.reset(new Render::Texture(desc, Render::ResourceState::PIXEL_SHADER_RESOURCE, HeapType::DEFAULT, residency_data));
    }
    {
        //  CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, tiles.x, tiles.y, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_TEXTURE_LAYOUT_UNKNOWN);
        // visibility_texture.reset(new Render::Texture(desc, Render::ResourceState::PIXEL_SHADER_RESOURCE, residency_data));
        visibility_texture.reset(new Render::ByteBuffer(mips[0].tiles.x * mips[0].tiles.y * mips[0].tiles.z * 4));
        std::vector<int> data;
        data.resize(mips[0].tiles.x * mips[0].tiles.y * mips[0].tiles.z, 255);
        visibility_texture->set_data(0, data);
        resolved_data.resize(data.size());
    }
    table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(2);
    residency_texture->texture_2d()->place_srv(table[0]);
    tiled_tex->texture_2d()->place_srv(table[1]);
  /*  sampler_table = Render::DescriptorHeapManager::get().get_samplers()->create_table(2);
    D3D12_SAMPLER_DESC wrapSamplerDesc = {};
    wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    wrapSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    wrapSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    wrapSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    wrapSamplerDesc.MinLOD = 0;
    wrapSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    wrapSamplerDesc.MipLODBias = 0.0f;
    wrapSamplerDesc.MaxAnisotropy = 1;
    wrapSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    wrapSamplerDesc.BorderColor[0] = wrapSamplerDesc.BorderColor[1] = wrapSamplerDesc.BorderColor[2] = wrapSamplerDesc.BorderColor[3] = 0;
    Render::Device::get().create_sampler(wrapSamplerDesc, sampler_table[0].cpu);
    wrapSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    Render::Device::get().create_sampler(wrapSamplerDesc, sampler_table[1].cpu);*/
    //	FileSystem::get().
    stream.reset(new std::ifstream(tiles_file_name, ios::binary));
    archive = ZipArchive::Create(*stream);
    heap_manager.flush_tilings(tiled_tex.get());
}

void TiledTexture::try_register()
{
    Asset::try_register();
}

TiledTexture::~TiledTexture()
{
    //data.clear();
}

void TiledTexture::load_tile(Tile::ptr& tile)
{
	UINT pixels = 1u<< tile->mip_level;
    int index = mips[tile->mip_level].tile_offset + tile->position.x + mips[tile->mip_level].tiles.x * tile->position.y + mips[tile->mip_level].tiles.y * mips[tile->mip_level].tiles.x * tile->position.z; // one_tile_size.x / pixels * (tile->position.y + tile->position.z * one_tile_size.y / pixels);
    auto entry = archive->GetEntry(index);
    tile->data.resize(entry->GetSize(), 255);

    if (entry)
    {
        //  std::lock_guard<std::mutex> g(file_mutex);
        std::ifstream s(tiles_file_name, ios::in | ios::binary);
        file_mutex.lock();
        auto stream = entry->GetDecompressionStream(s);
        file_mutex.unlock();
        stream->read((char*)const_cast<unsigned char*>(tile->data.data()), tile->data.size());
        entry->CloseDecompressionStream();
    }

    ivec3 mip_position = tile->position * pixels;
    int mip_index = mip_position.x + mip_position.y * residency_data->width + mip_position.z * residency_data->width * residency_data->height;
    {
        std::lock_guard<std::mutex> g(tile_mutex);
        add_tile(tile);

        for (UINT i = 0; i < pixels; i++)
            for (UINT j = 0; j < pixels; j++)
            {
                auto position = mip_index + i + j *  residency_data->width;
                residency_data->array[0]->mips[0]->data[position * 4] = std::min((int)(unsigned char)residency_data->array[0]->mips[0]->data[position * 4], tile->mip_level);
                residency_data->array[0]->mips[0]->data[position * 4 + 1] = residency_data->array[0]->mips[0]->data[position * 4];
                residency_data->array[0]->mips[0]->data[position * 4 + 2] = residency_data->array[0]->mips[0]->data[position * 4];
                residency_data->array[0]->mips[0]->data[position * 4 + 3] = 255;
            }

        residency_changed = true;
        tile->last_visible = step;
        upload_tiles.push_back(tile);
        visible_tiles.push_back(tile);
    }
}

void TiledTexture::load_group(const std::vector<std::list<Tile::ptr>>& queue, int i)
{
    std::vector<task<void>> tasks;

    for (auto& tile : queue[i])
    {
        tasks.emplace_back(create_task([tile, this]()
        {
            load_tile(const_cast<Tile::ptr&>(tile));
        }));
    }

    when_all(begin(tasks), end(tasks)).then([queue, this, i]()
    {
        if (i > 0)
            load_group(queue, i - 1);
    });
}

void TiledTexture::load_tilings()
{
    std::vector<std::list<Tile::ptr>> this_load_queue(mips.size());

    for (int i = 0; i < this_load_queue.size(); i++)
        std::swap(mips[i].load_queue, this_load_queue[i]);

    create_task([this_load_queue, this]()
    {
        for (size_t i = this_load_queue.size() - 1; i >= 0; i--)
        {
            for (auto& tile : this_load_queue[i])
                load_tile(const_cast<Tile::ptr&>(tile));
        }
    });
    //    load_group(this_load_queue, this_load_queue.size() - 1);
}

void TiledTexture::add_tile_to_load(ivec3 position, int mip_level)
{
    if (!is_good(position, mip_level)) return;

    auto& tile = get_tile(mip_level, position);

    if (tile) return;

    tile.reset(new Tile);
    tile->position = position;
    tile->mip_level = mip_level;
    tile->subresource = 0;
    mips[mip_level].load_queue.emplace_back(tile);
    add_tile_to_load(position / 2, mip_level + 1);
}

void TiledTexture::update(Render::CommandList::ptr& list)
{
    step++;
    //if (step % 100) return;
    std::lock_guard<std::mutex> g(tile_mutex);
    heap_manager.flush_tilings(tiled_tex.get());
 //   list->transition(visibility_texture.get(),  Render::ResourceState::COPY_SOURCE);
    list->get_copy().read_buffer(visibility_texture.get(), 0, visibility_texture->get_size(), [this](const char* data, UINT64 size)
    {
        std::vector<task<std::vector<std::pair<int, ivec3>>>> tasks;
        int thread_count = visibility_texture->get_desc().Height;
        int one_thread = int(size / thread_count / 4);
        memcpy(resolved_data.data(), data, size);
        //   auto int_data = reinterpret_cast<const unsigned int*>(data);

        // if (Application::get().is_alive())
        for (int thread = 0; thread < thread_count; thread++)
            tasks.emplace_back(create_task([this, thread, one_thread]()->std::vector<std::pair<int, ivec3>>
        {
            std::vector<std::pair<int, ivec3>> loads;

            for (int t = one_thread* thread; t < one_thread * (thread + 1); t++)
                // for (int t = 0; t < size / 4; t++)
            {
                int i = t % mips[0].tiles.x;
                int j = t / mips[0].tiles.y;
				unsigned int mip = resolved_data[t];
                int pixels = 1<< mip;
                ivec3 pos = { i / pixels, j / pixels, 0 };

                if (mip > 7) continue;

                // tile_mutex.lock();
                auto current_mip = (unsigned int)(residency_data_uploaded->array[0]->mips[0]->data[t * 4]);
                //  tile_mutex.unlock();
                //  auto tile = 	get_tile(mip, pos);

                if (current_mip > mip)
                    // load_tile(pos, mip);
                    loads.push_back(std::make_pair(mip, pos));
                else
                    make_tile_visible(mip, pos);
            }

            return loads;
        }
                                      ));
        when_all(begin(tasks), end(tasks)).then([tasks, this](std::vector<std::pair<int, ivec3>> r)
        {
            std::lock_guard<std::mutex> g(tile_mutex);

            for (auto it = visible_tiles.begin(); it != visible_tiles.end();)
            {
                if ((*it)->last_visible < step - 100)
                {
                    zero_tile((*it)->mip_level, (*it)->position);
                    it = visible_tiles.erase(it);
                }

                else
                    ++it;
            }

            for (auto && t : tasks)
                for (auto& p : t.get())
                {
                    add_tile_to_load(p.second, p.first);
                    add_tile_to_load(p.second + ivec3(1, 0, 0), p.first);
                    add_tile_to_load(p.second + ivec3(0, 1, 0), p.first);
                    add_tile_to_load(p.second + ivec3(1, 1, 0), p.first);
                }

            load_tilings();
        });
    });
//    list->transition(visibility_texture.get(),  Render::ResourceState::COPY_DEST);
    list->get_copy().update_buffer(visibility_texture.get(), 0, reinterpret_cast<char*>(clear_data.data()), UINT(clear_data.size() * 4));
 //   list->transition(visibility_texture.get(),  Render::ResourceState::COMMON);

    if (residency_changed)
    {
        memcpy(residency_data_uploaded->array[0]->mips[0]->data.data(), residency_data->array[0]->mips[0]->data.data(), residency_data->array[0]->mips[0]->data.size());
//        list->transition(residency_texture.get(),  Render::ResourceState::COPY_DEST);
        list->get_copy().update_texture(residency_texture, ivec3(0, 0, 0), ivec3(mips[0].tiles), 0, reinterpret_cast<const char*>(residency_data->array[0]->mips[0]->data.data()), residency_data->array[0]->mips[0]->width_stride);
 //       list->transition(residency_texture.get(),  Render::ResourceState::PIXEL_SHADER_RESOURCE);
        residency_changed = false;
    }

    if (upload_tiles.size())
    {
   //     list->transition(tiled_tex.get(), Render::ResourceState::COPY_DEST);

        for (auto& t : upload_tiles)
        {
            ivec3 gpu_size = one_tile_size;
            //   gpu_size.y = mips[t->mip_level].rows_per_tile;
            list->get_copy().update_texture(tiled_tex, t->position * gpu_size, gpu_size, t->mip_level, reinterpret_cast<const char*>(t->data.data()), mips[t->mip_level].stride_per_tile);
            t->data.clear();
        }

 //       list->transition(tiled_tex.get(),Render::ResourceState::PIXEL_SHADER_RESOURCE);
        upload_tiles.clear();
    }
}

template<class Archive>
void TiledTexture::serialize(Archive& ar, const unsigned int)
{
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(Asset);
    ar& NVP(file_name);
    ar& NVP(tiles_file_name);
    ar& NVP(mips);
    ar& NVP(one_tile_size);
    ar& NVP(sizes);
    ar& NVP(format);
    ar& NVP(format_size);

    if (Archive::is_loading::value)
        init();
}



template void AssetReference<TiledTexture>::serialize(serialization_oarchive& arch, const unsigned int version);
template void AssetReference<TiledTexture>::serialize(serialization_iarchive& arch, const unsigned int version);



template void TiledTexture::serialize(serialization_oarchive& arch, const unsigned int version);
template void TiledTexture::serialize(serialization_iarchive& arch, const unsigned int version);


BOOST_CLASS_EXPORT_IMPLEMENT(TiledTexture);
BOOST_CLASS_EXPORT_IMPLEMENT(AssetReference<TiledTexture>);

void TileHeapManager::place(Tile::ptr& tile)
{
    std::lock_guard<std::mutex> g(m);
	counter++;
    for (auto && p : pages)
    {
        if (p.place(tile))
            return;
    }

    pages.emplace_back();
    pages.back().place(tile);
}

void TileHeapManager::remove(Tile::ptr& tile, bool zero)
{
    std::lock_guard<std::mutex> g(m);
	counter--;
    if (!tile->page)
        return;

    tile->page->remove(tile);
   // pages.front().place_at(tile, zero_tile->tile_offset);

	if(zero)
	pages.front().zero_tile(tile);


}

TileHeapManager::TileHeapManager(Render::Resource* res):res(res)
{
	remove_all();

}

void TileHeapManager::flush_tilings(Render::Resource* res)
{
    std::lock_guard<std::mutex> g(m);

    for (auto && p : pages)
        p.flush_tilings(res);
}

void TileHeapManager::clear_all(ivec3 tiles, int mip_count)
{
    std::lock_guard<std::mutex> g(m);
    int mip = 1;

    for (int i = 0; i < mip_count; i++)
    {
        pages.front().place_region({ 0, 0, 0 }, { std::max(tiles.x / mip, 1) , std::max(tiles.y / mip, 1) , std::max(tiles.z / mip, 1) }, i, 0);
        mip *= 2;
    }
}

mip_info::mip_info(ivec3 tiles, unsigned int tile_offset)
{
    this->tiles = tiles;
    this->tile_offset = tile_offset;
    tile_positions.resize(tiles.x * tiles.y * tiles.z);
}
