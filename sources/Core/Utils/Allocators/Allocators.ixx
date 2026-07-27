export module Core:Allocators;

import :Debug;
import :Utils;
import :Math;
import stl.core;
import stl.threading;



export
{


	struct AllocationHandle;


	struct MemoryInfo
	{
		uint64 offset = 0;
		uint64 size = 0;
		//	uint64 aligned_offset;
		uint64 reset_id = 0;
		//	uint64 alloc_id;

		void* internal_data;
		bool operator==(const MemoryInfo& h)const = default;
		bool operator!=(const MemoryInfo& h)const = default;
		MemoryInfo() = default;
		MemoryInfo(const MemoryInfo& r) = default;
		MemoryInfo(uint64 offset, uint64 size,uint64 reset_id) :offset(offset), size(size),reset_id(reset_id)
		{

		}
	};


	class Allocator;

	class AllocatorHanle
	{
	protected:
		MemoryInfo info;
		Allocator* owner = nullptr;
	public:
		AllocatorHanle() = default;
		AllocatorHanle(const MemoryInfo& info, Allocator* owner);

		uint64 get_offset() const;

		const MemoryInfo& get_info() const
		{
			return info;
		}
		uint64 get_size() const;

		operator bool() const;

		uint64 get_reset_id()const;

		Allocator* get_owner() const;

		bool operator==(const AllocatorHanle& h) const;

		bool operator!=(const AllocatorHanle& h) const;

		void Free();
		void FreeAndClear();
	};



	class Allocator
	{
	public:
		THREAD_CHECKER
		template<class T = std::byte>
			AllocatorHanle Allocate(uint64 size, uint64 align = sizeof(T))
		{
			auto res = TryAllocate(size*sizeof(T), align);

			if (res)
				return *res;

			ASSERT(false);

			return AllocatorHanle();
		}


		virtual std::optional<AllocatorHanle>  TryAllocate(uint64 size, uint64 align = 1) = 0;

		virtual void Free(AllocatorHanle& handle) = 0;

		virtual bool isEmpty() const = 0;

		virtual void Reset() = 0;

		virtual uint64 get_max_usage() const = 0;
		virtual uint64 get_size() const = 0;
	};



	//shitty allocator, write a good one
	// dont want to write now

	class CommonAllocator :public Allocator
	{
		const uint64 start_region;
		const uint64 end_region;
		const uint64 size;
		uint64 max_usage;
		uint64 reset_id;
		struct block
		{
			uint64 begin;
			uint64 end;

			bool operator< (const block& b) const
			{
				return begin < b.begin;
			}
		};

		std::mutex m;
		std::set<block> free_blocks;
		std::map<uint64, const block*> fences;
		void check();

		uint64 merge_prev(uint64 start);
		uint64 merge_next(uint64 end);

	public:
		using Handle = AllocatorHanle;

		CommonAllocator(uint64 size = std::numeric_limits<uint64>::max());

		CommonAllocator(uint64 start_region, uint64 end_region);

		virtual ~CommonAllocator() = default;
		uint64 get_max_usage() const override;

		std::optional<Handle> TryAllocate(uint64 size, uint64 align = 1) override final;
		void Free(Handle& handle);

		void Reset() override;

		uint64 get_size() const override
		{
			return size;
		}

		bool isEmpty() const override
		{
			ASSERT_SINGLETHREAD


				if (free_blocks.size() == 1)
				{
					auto& b = *free_blocks.begin();
					return (b.begin == start_region) && (b.end == end_region - 1);

				}

			return false;
		}
	};

	// Per-lane progress watermarks describing "who has finished with this memory".
	// Lane = an independently progressing timeline (for GPU use: one per command
	// queue); the value is that lane's progress counter. `epoch` is the batch the
	// tag was recorded in (for GPU use: the frame), and 0 means "never owned".
	struct AllocSyncTag
	{
		static constexpr size_t MaxLanes = 4;

		uint64                     epoch = 0;
		std::array<uint, MaxLanes> lane{};

		// May a requester holding `requester` take memory last owned under *this*?
		// Never-owned memory and memory released in an earlier epoch are free for
		// all — callers are expected to sync epochs at the batch boundary. Within
		// the same epoch the requester must be at or past the owner on every lane.
		bool satisfied_by(uint64 current_epoch, const AllocSyncTag& requester) const
		{
			if (epoch == 0 || epoch < current_epoch) return true;

			for (size_t i = 0; i < MaxLanes; i++)
				if (lane[i] > requester.lane[i]) return false;

			return true;
		}

		// Conservative combination, for when two tagged ranges become one.
		void join(const AllocSyncTag& o)
		{
			epoch = std::max(epoch, o.epoch);

			for (size_t i = 0; i < MaxLanes; i++)
				lane[i] = std::max(lane[i], o.lane[i]);
		}
	};


	// First-fit block allocator whose free blocks remember WHEN their previous owner
	// finished with them, so a range is only ever handed to a requester guaranteed
	// to run after that point. Built for GPU transient memory: recycling a range for
	// a consumer on another queue with no fence in between corrupts memory and faults
	// the device, and the usual fix (inserting a barrier) is exactly what we want to
	// avoid — so an unusable block is simply skipped and the search moves on, which
	// lands the caller on other memory (or, via PagedAllocator, a fresh page).
	//
	// Self-contained on purpose: one instance owns one contiguous region (in practice
	// one page), so a block is identified by its offset alone and no external heap
	// identity has to be plumbed in. Tags live on the blocks and travel through split
	// and coalesce, so unlike a bolt-on filter this cannot be defeated by the free
	// list being merged behind its back.
	class SyncAwareAllocator : public Allocator
	{
	public:
		using Handle = AllocatorHanle;

	private:
		struct block
		{
			uint64       begin = 0;
			uint64       end   = 0;   // inclusive
			AllocSyncTag tag;

			bool operator<(const block& b) const { return begin < b.begin; }
		};

		const uint64 start_region;
		const uint64 end_region;
		const uint64 region_size;

		uint64 max_usage = 0;
		uint64 reset_id  = 0;

		std::set<block> free_blocks;

		void reset_free_list()
		{
			free_blocks.clear();
			if (start_region < end_region)
				free_blocks.insert(block{ start_region, end_region - 1, AllocSyncTag{} });
		}

		// Two ranges may only be merged when they agree on whether they are still
		// restricted this epoch. Merging restricted memory into unconstrained memory
		// would drag the whole block down to the restricted tag and throw reuse away;
		// merging the other way would be unsafe.
		bool same_class(const AllocSyncTag& a, const AllocSyncTag& b) const
		{
			return (a.epoch >= s_epoch) == (b.epoch >= s_epoch);
		}

		// First-fit scan, skipping blocks this requester is not ordered after.
		std::optional<Handle> try_place(uint64 size, uint64 align)
		{
			for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
			{
				const block cur = *it;

				uint64 aligned = Math::roundUp(cur.begin, align);
				if (aligned < cur.begin || aligned + size - 1 > cur.end) continue;

				// Skip memory this requester is not ordered after. Moving on is the
				// whole point: it relocates the allocation instead of forcing a
				// barrier onto the caller.
				if (!cur.tag.satisfied_by(s_epoch, s_request)) continue;

				free_blocks.erase(it);

				// Remainders are still the same previously-owned memory, so they keep
				// the tag.
				if (aligned > cur.begin)
					free_blocks.insert(block{ cur.begin, aligned - 1, cur.tag });

				if (aligned + size - 1 < cur.end)
					free_blocks.insert(block{ aligned + size, cur.end, cur.tag });

				max_usage = std::max(max_usage, aligned + size);
				return Handle(MemoryInfo(aligned, size, reset_id), this);
			}

			return std::nullopt;
		}

		// Merge every adjacent pair that is now in the same class. Blocks released in
		// an earlier epoch have since become unconstrained, so neighbours that could
		// not be merged at release time usually can be by now. Run only when nothing
		// fit — recovering a contiguous range here is much cheaper than letting the
		// caller fall through and commit a whole new page.
		bool collapse()
		{
			if (free_blocks.size() < 2) return false;

			std::set<block> merged;
			bool changed = false;

			auto it = free_blocks.begin();
			block cur = *it++;

			for (; it != free_blocks.end(); ++it)
			{
				if (cur.end + 1 == it->begin && same_class(cur.tag, it->tag))
				{
					cur.end = it->end;
					cur.tag.join(it->tag);
					changed = true;
				}
				else
				{
					merged.insert(cur);
					cur = *it;
				}
			}

			merged.insert(cur);

			if (changed) free_blocks.swap(merged);
			return changed;
		}

	public:
		// PagedAllocator owns one instance per page and chooses between them
		// internally, so the request being served cannot be threaded through the
		// virtual TryAllocate signature. These carry it instead; single-threaded by
		// construction (one batch/frame is prepared at a time).
		static inline uint64       s_epoch   = 1;
		static inline AllocSyncTag s_request;   // consulted by TryAllocate
		static inline AllocSyncTag s_release;   // stamped onto blocks by Free

		// Start a new batch. Everything released earlier becomes unconstrained,
		// which also lets it coalesce freely again — no sweep over blocks needed.
		static void begin_epoch() { ++s_epoch; }

		// Default is deliberately the most restrictive thing possible: a forgotten
		// set_request costs memory (nothing in-epoch is reusable), never safety.
		static void set_request(const AllocSyncTag& t) { s_request = t; }

		static void set_release(const AllocSyncTag& t) { s_release = t; }

		static void clear_request()
		{
			s_request = AllocSyncTag{};
		}

		SyncAwareAllocator(uint64 size = std::numeric_limits<uint64>::max())
			: start_region(0), end_region(size), region_size(size)
		{
			reset_free_list();
		}

		SyncAwareAllocator(uint64 start_region, uint64 end_region)
			: start_region(start_region), end_region(end_region), region_size(end_region - start_region)
		{
			reset_free_list();
		}

		uint64 get_max_usage() const override { return max_usage; }
		uint64 get_size()      const override { return region_size; }

		std::optional<Handle> TryAllocate(uint64 size, uint64 align = 1) override final
		{
			if (size == 0)
				return Handle(MemoryInfo(start_region, 0, reset_id), this);

			if (auto handle = try_place(size, align)) return handle;

			// Nothing fit — try recovering contiguity before the caller gives up on
			// this region and commits a new page.
			if (collapse()) return try_place(size, align);

			return std::nullopt;
		}

		void Free(Handle& handle) override
		{
			if (!handle) return;
			if (handle.get_reset_id() != reset_id) return;

			ASSERT(handle.get_owner() == this);
			if (!handle.get_size()) return;

			block freed;
			freed.begin = handle.get_info().offset;
			freed.end   = freed.begin + handle.get_size() - 1;
			freed.tag   = s_release;

#ifdef DEV
			for (auto& b : free_blocks)
				ASSERT(b.end < freed.begin || b.begin > freed.end);   // no double free / overlap
#endif

			// Coalesce with same-class neighbours only (see same_class). Blocks kept
			// apart here rejoin later via collapse(), once begin_epoch() has made them
			// all unconstrained.
			auto next = free_blocks.lower_bound(block{ freed.end + 1, 0, AllocSyncTag{} });

			if (next != free_blocks.end() && next->begin == freed.end + 1 && same_class(freed.tag, next->tag))
			{
				freed.end = next->end;
				freed.tag.join(next->tag);
				next = free_blocks.erase(next);
			}

			if (next != free_blocks.begin())
			{
				auto prev = std::prev(next);

				if (prev->end + 1 == freed.begin && same_class(freed.tag, prev->tag))
				{
					freed.begin = prev->begin;
					freed.tag.join(prev->tag);
					free_blocks.erase(prev);
				}
			}

			free_blocks.insert(freed);
		}

		void Reset() override
		{
			reset_id++;
			max_usage = 0;
			reset_free_list();
		}

		bool isEmpty() const override
		{
			if (free_blocks.size() != 1) return false;

			auto& b = *free_blocks.begin();
			return b.begin == start_region && b.end == end_region - 1;
		}
	};


	class LinearAllocator : public Allocator
	{

		const uint64 start_region;
		const uint64 end_region;

	protected:
		uint64 offset;
		const uint64 size;
	public:
		using Handle = AllocatorHanle;

		LinearAllocator(uint64 size = std::numeric_limits<uint64>::max());
		LinearAllocator(uint64 start_region, uint64 end_region);
		uint64 get_max_usage() const override;
		bool isEmpty() const override;
		void Free(Handle& handle);
		uint64 get_size() const override;
		std::optional<Handle> TryAllocate(uint64 size, uint64 align = 1) override final;
		void Reset() override;
	};


	template<class T> class TypedHandle;
	template<class T>
	class DataProvider
	{
		friend class TypedHandle<T>;
		virtual void Free(TypedHandle<T>& handle) = 0;
	public:
		virtual	T& aquire(uint64 offset) {
			return aquire(offset, 1)[0];
		}

		virtual	std::span<T> aquire(uint64 offset, uint64 size) = 0;

		virtual	void write(uint64 offset, std::vector<T>& v) = 0;
	};





	template<class T>
	class TypedHandle
	{
		DataProvider<T>* provider = nullptr;
	public:

		AllocatorHanle handle;

		TypedHandle() = default;
		TypedHandle(AllocatorHanle& handle, DataProvider<T>* provider) : handle(handle), provider(provider)
		{
			//ASSERT(handle.get_size());
		}
		operator bool()
		{
			return provider;
		}

		uint64 get_offset() const
		{
			return handle.get_offset();
		}

		uint64 get_size() const
		{
			return handle.get_size();
		}

			uint64 get_offset_in_bytes() const
		{
			return handle.get_offset()*sizeof(T);
		}

		uint64 get_size_in_bytes() const
		{
			return handle.get_size()*sizeof(T);
		}


		std::span<T> aquire()
		{
			return provider->aquire(handle.get_offset(), handle.get_size());
		}

		std::span<T> aquire_one(uint64 offset)
		{
			return provider->aquire(handle.get_offset() + offset, 1);
		}

		std::span<T> aquire(uint64 size)
		{
			return provider->aquire(handle.get_offset(), size);
		}

		void write(uint64 offset, std::vector<T>& r)
		{
			return provider->write(handle.get_offset() + offset, r);
		}

		std::vector<T> map()
		{
			std::vector<T> t;
			t.resize(handle.get_size());
			return t;
		}

		std::vector<T> map(uint64 offset)
		{
			std::vector<T> t;
			t.resize(1);
			return t;
		}

		void Free()
		{
			if (provider) provider->Free(*this);
		}
	};
	template<class T, class AllocatorType>
	class DataAllocator : private AllocatorType, public DataProvider<T>
	{
		std::mutex m;
	public:
		using Handle = TypedHandle<T>;

		DataAllocator(uint64 count) :AllocatorType(count)
		{

		}

		Handle Allocate(uint64 count)
		{
			std::lock_guard<std::mutex> g(m);
			auto handle = AllocatorType::Allocate(count);
			return Handle(handle, this);
		}

		void Free(Handle& handle)
		{
			std::lock_guard<std::mutex> g(m);

			on_free(handle.get_offset(), handle.get_offset() + handle.get_size());
			handle.handle.Free();
		}

		uint64 get_max_usage()
		{
			return AllocatorType::get_max_usage();
		}

		virtual void moveBlock(uint64 from, uint64 to) {

		}

		virtual void on_free(uint64 from, uint64 to) {

		}

	};
}
