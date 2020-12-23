#pragma once

class HeapPage;

struct Tile
{
    using ptr = std::shared_ptr<Tile>;


    ivec3 position;
    UINT subresource;
    int mip_level;
    int last_visible = 0;
    TileState state = TileState::NONE;
    std::vector<unsigned char> data;
	int childs_count = 0;

    HeapPage* page = nullptr;
    UINT tile_offset = 0;


	bool has_static = false;
	bool has_dynamic = false;



	HeapPage* page_backup = nullptr;
	UINT tile_offset_backup = 0;
};


class HeapPage
{
        ComPtr<ID3D12Heap>tile_heap;
        unsigned int heap_count = 0;
        std::list<unsigned int> free_tiles;
        unsigned int max_count;
   
		std::vector<D3D12_TILED_RESOURCE_COORDINATE> startCoordinates;
		std::vector<D3D12_TILE_REGION_SIZE> regionSizes;
		std::vector<D3D12_TILE_RANGE_FLAGS> rangeFlags;
		std::vector<UINT> heapRangeStartOffsets;
		std::vector<UINT> rangeTileCounts;

        unsigned int allocate();

		std::vector<Render::Resource::ptr> tiles;

    public:

        static const unsigned int INVALID_VALUE = -1;
        HeapPage();
		~HeapPage()
		{
			//Render::Device::get().unused(tile_heap, ResourceHandle());
		}

		Render::Resource::ptr get_tile_texture(int offset)
		{
			return tiles[offset];
		}
		ComPtr<ID3D12Heap> get_native()
		{
			return tile_heap;
		}
        bool place(Tile::ptr& tile);
        void remove(Tile::ptr& tile);


        void place_at(Tile::ptr& tile, unsigned int offset);
		void zero_tile(Tile::ptr& tile);

        void flush_tilings(Render::CommandList& list, Render::Resource* res);

		void clear(Render::CommandList& list)
		{

            for(auto &t:tiles)
			list.clear_uav(t, t->create_view<Render::TextureView>(*list.frame_resources).rwTexture2D);
		}

        void place_region(ivec3 pos, ivec3 size, unsigned int subresource, unsigned int offset);
};






class TileHeapManager
{

        std::list<HeapPage> pages;
        std::mutex m;
		int counter = 0;
	
	
    public:
        void place(Tile::ptr& tile);

        void remove(Tile::ptr& tile, bool zero = true);

        TileHeapManager(Render::Resource* res= nullptr);
		Render::Resource* res = nullptr;

		void remove_all(Render::CommandList& list)
		{
			std::lock_guard<std::mutex> g(m);

			counter = 0;
			//pages.emplace_back();

			if (!res) return;
			UINT num_tiles = 1;
			D3D12_PACKED_MIP_INFO mip_info;
			D3D12_TILE_SHAPE tile_shape;
			UINT num_sub_res = 20;
			//UINT first_sub_res;
			D3D12_SUBRESOURCE_TILING tilings[20];
			Render::Device::get().get_native_device()->GetResourceTiling(res->get_native().Get(), &num_tiles, &mip_info, &tile_shape, &num_sub_res, 0, tilings);
			update_tiling_info info;

			for (int i = 0; i < mip_info.NumStandardMips; i++)
			{
                ResourceTile tile;
                tile.size = { tilings[i].WidthInTiles,tilings[i].HeightInTiles,tilings[i].DepthInTiles };
                tile.subresource = i;
                info.add_tile(tile);
			}

			
			info.resource = res;
            list.update_tilings(std::move(info));


			for (auto& p : pages)
			{
				list.tracked_heaps.push_back(p.get_native());
			}
			pages.clear();


		}
        void flush_tilings(Render::CommandList& list, Render::Resource* res);

        void clear_all(ivec3 tiles, int mip_count);

		void clear(Render::CommandList& list)
		{
			for (auto&& e : pages)
			{
				e.clear(list);
			}
		}

		size_t get_used_tiles()
		{
			return	pages.size()*128;

		}


};

struct mip_info
{
        ivec3 tiles;
        unsigned int tile_offset;
        std::vector<Tile::ptr> tile_positions;
        std::list<Tile::ptr> load_queue;
		UINT rows_per_tile;
        UINT stride_per_tile;

        mip_info(ivec3 tiles, unsigned int tile_offset);

    private:
        mip_info() = default;
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(tiles);
            ar& NVP(tile_offset);
            ar& NVP(rows_per_tile);
            ar& NVP(stride_per_tile);

            if (Archive::is_loading::value)
                tile_positions.resize(tiles.x * tiles.y * tiles.z);
        }

};

class TiledTexture : public Asset
{

        std::vector<mip_info> mips;
        ivec3 one_tile_size;
        //ivec3 one_tile_size_gpu;

        std::vector<unsigned int> clear_data;

        std::list<Tile::ptr> upload_tiles;
        std::list<Tile::ptr> visible_tiles;

        std::mutex tile_mutex;
        std::mutex file_mutex;

        std::shared_ptr<std::ifstream> stream;
        ZipArchive::Ptr archive;

        TileHeapManager heap_manager;


        Render::Texture::ptr null_tile;

        texture_data::ptr  residency_data;

        texture_data::ptr  residency_data_uploaded;

        std::vector<unsigned int> resolved_data;
        long step = 0;
        bool residency_changed = false;


        Tile::ptr& get_tile(int mip_level, ivec3 pos);

        void clear_tilings();

        void make_tile_visible(int mip_level, ivec3 pos);
        void zero_tile(int mip_level, ivec3 position);

        void zero_tile(Tile::ptr& tile);


        void add_tile(Tile::ptr tile);




        bool is_good(ivec3 pos, int mip);
        std::string file_name;
        std::string tiles_file_name;
        ivec3 sizes;
        UINT format_size = 4;
        DXGI_FORMAT format;
    public:
        using ptr = std::shared_ptr<TiledTexture>;
        using ref = AssetReference<TiledTexture>;
        Render::Texture::ptr tiled_tex;
        Render::Texture::ptr residency_texture;
        Render::ByteBuffer::ptr visibility_texture;

        Render::HandleTable table;
        Render::HandleTable sampler_table;
        virtual Asset_Type get_type() override;
        TiledTexture(std::string file_name);

        TiledTexture();
        void init();
        virtual void try_register();


        virtual ~TiledTexture();

        void load_tile(Tile::ptr& tile);
        void load_group(const std::vector<std::list<Tile::ptr>>& queue, int i);
        void load_tilings();

        void add_tile_to_load(ivec3 position, int mip_level);


        void update(Render::CommandList::ptr& list);
    private:

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int);

};

BOOST_CLASS_EXPORT_KEY2(TiledTexture, "TiledTexture");

BOOST_CLASS_EXPORT_KEY(AssetReference<TiledTexture>);