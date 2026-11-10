export module Graphics:TextureStreaming;

import Core;
import HAL;
import :Texture;
import :Streaming;

// Texture streaming: a client of the Streaming::Streamer, like Quanta's pages, with the tiles of a texture's mips as pages.
//
// A streamed texture is a reserved (tiled) texture: its address space is there from the start, but a 64 KB tile of a mip
// has memory behind it only while it is resident. Tiles come from one pool shared by all streamed textures (the
// streamer's slots, each a tile of a heap), so the pool size is the texture memory budget.
//
//   pages     one per tile of every standard mip. The mips too small to fill a tile (the packed tail) are not pages:
//             they are mapped and filled when the texture is added and stay.
//   parents   a tile's parent is the tile of the next coarser mip that covers it, so a mip is never shown without the
//             coarser one under it. The coarsest standard mip is the root: always resident, what a missing tile falls
//             back to.
//   show/hide the page table is the tile mapping itself: show() maps the tile to its pool slot and copies its texels in,
//             hide() unmaps it. Both are recorded by prepare() into the frame's list, in the order the streamer made them.
//
// min_mip() is the finest mip that is complete together with every coarser one. A shader may sample down to it and no
// further: an unmapped tile reads as zero. It only moves when prepare() records the change that causes it, so a frame
// recorded after that prepare never sees a mip the GPU has not got.

export namespace TextureStreaming
{
	constexpr uint32_t TileBytes = 64 * 1024;

	// What the material pixel shaders paint over the albedo, to see the streaming at work. The values are the ones
	// QuantaFeedback::stream_debug carries to the shader.
	enum class DebugView : uint32_t
	{
		Off = 0,
		SampledMip = 1,    // the mip a pixel really got (the finer of what it wanted and what is resident): blue = 0, red = coarse
		MissingMips = 2,   // how many mips short of what it wanted: green = none, red = 4 or more
	};

	class Manager;

	// The tiles of a texture, as they sit in a partition of its asset file: every standard mip's tiles in CopyTiles layout
	// (64 KB each, x fastest) one after the other, then the mips too small for a tile as plain rows. Any tile is one range of
	// the file, so it can be read alone and handed to the GPU without cutting anything out of a CPU copy of the texture.
	//
	// The tile bytes are the GPU's own swizzle, which this assumes is the same on every GPU the asset is used on (see
	// HAL::StorageRequest::Tiles). Asked for with Manager::bake(), which makes them on the GPU that saves the asset.
	class TileFile : public HAL::texture_data_header
	{
	public:
		struct Mip
		{
			uint32_t tiles_x = 0;
			uint32_t tiles_y = 0;
			uint64_t offset = 0;   // in the file, of the first tile; set by loading
		};

		struct Packed
		{
			uint32_t width = 0;
			uint32_t height = 0;
			uint32_t row_stride = 0;
			uint64_t size = 0;
			uint64_t offset = 0;   // set by loading
		};

		std::vector<Mip> mips;       // the standard mips, finest first
		std::vector<Packed> packed;  // the mips below a tile

		std::filesystem::path path;  // set by loading

		// What is written, kept only until the asset is saved.
		std::vector<std::vector<std::byte>> tile_bytes;     // per standard mip
		std::vector<std::vector<uint8_t>> packed_bytes;

		// Reads one tile (TileBytes) or bytes of the file the tiles were loaded from.
		bool read_tile(uint32_t mip, uint32_t x, uint32_t y, void* destination);
		bool read(uint64_t offset, void* destination, size_t size);

		// Where one tile is: the file and the offset in it, for a read that does not go through this object (DirectStorage).
		void locate_tile(uint32_t mip, uint32_t x, uint32_t y, std::filesystem::path& file, uint64_t& offset);

		// The asset file was written again: `fresh` is the same tiles as loaded from the new file. Readers wait for it.
		void relocate(const TileFile& fresh);

		// Fills tile_bytes and packed_bytes from the file, to write them into another one.
		bool read_all();

	private:
		std::mutex file_mutex;
		std::fstream stream;

		bool read_unlocked(uint64_t offset, void* destination, size_t size);

		SERIALIZE()
		{
			SAVE_PARENT(texture_data_header);

			uint32_t mip_count = static_cast<uint32_t>(mips.size());
			uint32_t packed_count = static_cast<uint32_t>(packed.size());
			ar& NVP(mip_count);
			ar& NVP(packed_count);

			if constexpr (Archive::is_loading::value)
			{
				mips.resize(mip_count);
				packed.resize(packed_count);
			}

			for (auto& mip : mips)
			{
				ar& NVP(mip.tiles_x);
				ar& NVP(mip.tiles_y);
			}
			for (auto& p : packed)
			{
				ar& NVP(p.width);
				ar& NVP(p.height);
				ar& NVP(p.row_stride);
				ar& NVP(p.size);
			}

			if constexpr (Archive::is_loading::value)
			{
				UniversalContext& context = cereal::get_user_data<UniversalContext>(ar);
				std::fstream* file = context.template get_context<std::fstream*>();
				path = context.template get_context<std::filesystem::path>();

				// The bytes stay in the file: only where they are is kept.
				uint64_t at = file->tellg();
				for (auto& mip : mips)
				{
					mip.offset = at;
					at += uint64_t(mip.tiles_x) * mip.tiles_y * TileBytes;
				}
				for (auto& p : packed)
				{
					p.offset = at;
					at += p.size;
				}
				file->seekg(static_cast<std::streamoff>(at), std::ios::beg);
			}
			else
			{
				for (size_t i = 0; i < mips.size(); i++)
				{
					ASSERT(tile_bytes[i].size() == uint64_t(mips[i].tiles_x) * mips[i].tiles_y * TileBytes);
					ar& NVP(cereal::binary_data(tile_bytes[i].data(), tile_bytes[i].size()));
				}
				for (size_t i = 0; i < packed.size(); i++)
				{
					ASSERT(packed_bytes[i].size() == packed[i].size);
					ar& NVP(cereal::binary_data(packed_bytes[i].data(), packed_bytes[i].size()));
				}
			}
		}
	};

	class Streamed : public Streaming::Resource
	{
		friend class Manager;

		struct Mip
		{
			uint32_t first_page = 0;
			uint32_t tiles_x = 0;
			uint32_t tiles_y = 0;
			uint32_t resident = 0;   // tiles shown (guarded by the streamer's mutex)
		};

		std::vector<Mip> mips;                 // the standard mips, finest first
		std::vector<uint32_t> views;           // descriptor-heap indices of the SRVs of this texture the shaders use
		uint32_t base_id = 0;
		std::atomic<uint32_t> published_mip{ 0 };

		// The finest mip that is always resident (with every coarser one): what a missing tile falls back to, and the
		// quality ray tracing is guaranteed, since its samples report nothing to the streamer.
		uint32_t root_mip = 0;

		// What the draws last sampled (Manager::wanted), guarded by the streamer's mutex.
		uint32_t wanted_mip = 0;
		uint64_t last_seen = 0;   // the feedback frame it was reported in, 0 = never

		// The finest mip complete together with every coarser one, counting what is shown now.
		uint32_t complete_mip() const;

	public:
		HAL::Texture::ptr texture;             // reserved
		HAL::texture_data::ptr source;         // the texels the tiles are cut from, or null when `file` has them
		std::shared_ptr<TileFile> file;        // the tiles, read from an asset file

		uint32_t standard_mips() const { return static_cast<uint32_t>(mips.size()); }
		uint32_t page_count() const { return static_cast<uint32_t>(states.size()); }

		uint32_t page_of(uint32_t mip, uint32_t x, uint32_t y) const { return mips[mip].first_page + y * mips[mip].tiles_x + x; }

		// Where a page is: its mip and tile coordinates.
		void locate(uint32_t page, uint32_t& mip, uint32_t& x, uint32_t& y) const;

		// How many pages (tiles) a mip has.
		uint32_t tiles_in_mip(uint32_t mip) const { return mips[mip].tiles_x * mips[mip].tiles_y; }

		uint32_t min_mip() const { return published_mip.load(std::memory_order_relaxed); }

		// The mip the shaders are told to stay at or above: min_mip(), or the Manager's mip limit if that is coarser.
		// Never coarser than the root, which is always resident.
		uint32_t shader_mip() const;

		uint64_t page_id_base() const override { return base_id; }
		bool is_root(uint32_t page) const override { return page >= mips[root_mip].first_page; }
		uint32_t resident_floor_mip() const { return root_mip; }

		~Streamed() override;
	};

	class Manager : public Singleton<Manager>, private Streaming::Backend
	{
		friend class Streamed;

		static constexpr uint32_t MaxPageIds = 1u << 18;
		// One entry per descriptor of the shader-visible heap (HAL.DescriptorHeap.cpp).
		static constexpr uint32_t ViewCapacity = 65536 * 8;

		// Everything one prepare() records for one change of one texture, in the order the streamer made the changes.
		struct Copy
		{
			uint32_t subresource = 0;
			ivec3 offset;
			ivec3 box;
			uint32_t row_stride = 0;
			bool tiles = false;   // `data` is one tile in CopyTiles layout, not rows of texels
			std::vector<uint8_t> data;
		};

		struct Op
		{
			enum class Kind { Map, Unmap, Packed };

			Kind kind = Kind::Map;
			Streamed* owner = nullptr;
			HAL::Texture::ptr texture;   // keeps the GPU resource alive until the change is recorded
			uint32_t mip = 0;
			uint32_t x = 0;
			uint32_t y = 0;
			uint32_t slot = 0;
			bool premapped = false;   // a Map that begin_load already made (a DirectStorage tile load maps before it reads)
			std::vector<Copy> copies;
		};

		std::shared_ptr<Streaming::Streamer> streamer;
		std::mutex& m;

		std::vector<HAL::TileHeapPosition> pool;                  // slot -> a tile of a heap
		std::unordered_map<uint32_t, Op> staged;                  // loaded, waiting to be shown: by page id
		std::vector<Op> ops;                                      // shown or hidden since the last prepare()

		struct IdRange { uint32_t first; uint32_t count; };
		std::vector<IdRange> free_ids;
		uint32_t next_id = 0;

		// The values stream_info is to get, in order, recorded by the next prepare().
		struct InfoWrite
		{
			uint32_t view;
			uint32_t value;
		};
		std::vector<InfoWrite> info_writes;
		std::unordered_map<const HAL::Resource*, Streamed*> by_resource;

		uint32_t pool_tiles = 1024;
		int applied_limit = 0;   // the mip limit stream_info was last written for
		uint64_t texture_count = 0;
		uint64_t feedback_frame = 0;   // counts collect_feedback() calls, guarded by the streamer's mutex

		uint32_t allocate_ids(uint32_t count);
		void free_id_range(uint32_t first, uint32_t count);
		void release(Streamed& streamed);
		void apply_pool_size();

		std::shared_ptr<Streamed> create_streamed(const HAL::texture_data_header& header, const HAL::texture_data::ptr& data, const std::shared_ptr<TileFile>& file, const std::wstring& name);

		// A file tile on its way to its slot through DirectStorage (begin_direct_load).
		struct DirectTileLoad
		{
			HAL::Resource::ptr resource;
			HAL::StorageRequest request;
			HAL::FenceWaiter mapped;
			HAL::FenceWaiter done;
			bool requested = false;

			bool poll();
			~DirectTileLoad();
		};

		Streaming::Backend::Load begin_direct_load(Streamed& streamed, uint32_t page, uint32_t slot);
		Op make_map(Streamed& streamed, uint32_t page, uint32_t slot);
		static void cut_tile(const Streamed& streamed, uint32_t mip, uint32_t x, uint32_t y, Copy& out);

		// Streaming::Backend
		void allocate_pool(uint32_t count) override;
		void release_slot(uint32_t slot) override;
		void restore_slot(uint32_t slot) override;
		Streaming::Backend::Load begin_load(Streaming::Resource& resource, uint32_t page, uint32_t slot, bool allow_file) override;
		void show(Streaming::Resource& resource, uint32_t page, uint32_t slot) override;
		void hide(Streaming::Resource& resource, uint32_t page) override;
		void set_flags(Streaming::Resource& resource, uint32_t page, uint32_t flags, uint32_t shown_epoch) override;

	public:
		// One dword per descriptor-heap index, read by the material shaders (MaterialInfo::stream_info): 0 for a texture
		// that is not streamed, otherwise min_mip() + 1 of the streamed texture whose SRV sits at that index. A shader
		// samples no finer than that.
		HAL::StructuredBufferView<uint> stream_info;

		// Written by the main view's material pixel shaders (QuantaFeedback::textures, stream_feedback.hlsl): per
		// descriptor-heap index, 16 minus the finest mip a sample asked for this frame, 0 = not sampled.
		HAL::StructuredBufferView<uint> wanted;

		Manager();
		~Manager() override;

		// Tells the shaders that the descriptor at `view_index` is an SRV of `resource` (a streamed texture's: anything
		// else is ignored), so that they clamp what they sample to its resident mips. A texture may have several views.
		void bind_view(const HAL::Resource* resource, uint32_t view_index);

		// Whether this device can map tiles of textures (reserved resources).
		static bool supported();

		// Properties-panel knobs ("Texture streaming"). The mip limit is the finest mip any streamed texture may use: raising
		// it stops the finer tiles being asked for and makes the shaders sample no finer at once, to see how the system
		// reacts. A texture with fewer mips than the limit stays at its coarsest standard mip.
		static int mip_limit();
		static DebugView debug_view();

		// The largest side, in texels, of the mips every streamed texture keeps resident (at least its coarsest standard
		// mip). Read when a texture is created, so it applies to textures made afterwards.
		static int resident_size();
		static void set_resident_size(int texels);

		// The most tiles the streamed textures hold together (roots included). Takes effect from the next texture.
		void set_pool_tiles(uint32_t tiles);

		// A reserved texture made from `data`, with its coarsest mips resident and the rest to be asked for.
		std::shared_ptr<Streamed> create(const HAL::texture_data::ptr& data, const std::wstring& name = L"streamed texture");

		// The same from the tiles of an asset file: nothing of the texture stays in memory, and each tile is read from the file
		// when it is wanted.
		std::shared_ptr<Streamed> create(const std::shared_ptr<TileFile>& file, const std::wstring& name = L"streamed texture");

		// The tiles of `data` in CopyTiles layout, ready to be saved: made on this GPU by filling a reserved texture and
		// reading its tiles back. Slow, once per asset.
		std::shared_ptr<TileFile> bake(const HAL::texture_data::ptr& data);

		// What the draw needs of a texture: the mips from `mip` down to the coarsest. Its tiles are asked for; the tiles of
		// finer mips are offered to the streamer for eviction, to be dropped only if something else needs their slots.
		struct Demand
		{
			const Streamed* texture;
			uint32_t mip;
		};

		// One request set for all the textures given (the streamer works on a set at a time, and an eviction offer is only
		// good for the set it comes with).
		void request(std::span<const Demand> demands);
		void request(const Streamed& streamed, uint32_t mip) { request(std::span<const Demand>(std::array<Demand, 1>{ Demand{ &streamed, mip } })); }

		using Stats = Streaming::Stats;
		Stats stats();

		// Frames a texture keeps the mip it was last seen at after the draws stop sampling it; then only its roots are wanted.
		static constexpr uint64_t HoldFrames = 120;

		// Reads and clears what the draws reported (recorded in `list`, which runs before this frame's draws, so it holds
		// the previous frame's), to be applied by request_from_feedback() once it has come back from the GPU.
		void collect_feedback(HAL::CommandList::ptr& list);

		// One request set from what the draws reported: each seen texture at the finest mip they sampled, the others at
		// their roots. Offered mips above that are the streamer's to evict.
		void request_from_feedback();

		// Once per frame: records the tile mappings and texel copies of what the streamer shown or hid since the last call.
		void prepare(HAL::CommandList::ptr& list);
	};
}
