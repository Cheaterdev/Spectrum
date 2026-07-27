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



	// Defined below. Only ever referenced through a pointer/reference here, so the
	// declaration is enough.
	struct AllocSyncTag;

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

		// Constraint-aware entry points. `constraint` describes what the requester is
		// ordered after, and `tag` records what the releaser had finished, so an
		// allocator can refuse to hand memory to someone who would race its previous
		// owner. The defaults ignore both, so allocators with no such notion need no
		// changes; only callers that care pass them.
		virtual std::optional<AllocatorHanle> TryAllocate(uint64 size, uint64 align, const AllocSyncTag* constraint)
		{
			return TryAllocate(size, align);
		}

		virtual void Free(AllocatorHanle& handle, const AllocSyncTag& tag)
		{
			Free(handle);
		}

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

		// Recorded in an epoch older than the requester's (or never owned at all), so
		// permanently safe for everyone from here on. Callers are expected to fully
		// order epochs against each other — for GPU use the frame boundary already
		// waits across queues.
		bool stale_for(const AllocSyncTag& requester) const
		{
			return epoch == 0 || epoch < requester.epoch;
		}

		// May a requester holding `requester` take memory last owned under *this*?
		// Inside one epoch the requester must be at or past the owner on every lane.
		bool satisfied_by(const AllocSyncTag& requester) const
		{
			if (stale_for(requester)) return true;

			for (size_t i = 0; i < MaxLanes; i++)
				if (lane[i] > requester.lane[i]) return false;

			return true;
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
		// --- FIT: an ordinary coalescing free list. No tags, merges unconditionally;
		// reads exactly like any other first-fit allocator.
		struct span
		{
			uint64 begin = 0;
			uint64 end   = 0;   // inclusive

			bool operator<(const span& s) const { return begin < s.begin; }
		};

		const uint64 start_region;
		const uint64 end_region;
		const uint64 region_size;

		uint64 max_usage = 0;
		uint64 reset_id  = 0;

		std::set<span> free_spans;

		// --- SAFETY: who last owned which bytes, recorded per released range and kept
		// deliberately apart from the free list. Because merging free space cannot
		// touch these, the two concerns never interfere: the free list is free to
		// coalesce as aggressively as it likes, and a candidate placement is simply
		// checked against whatever records it overlaps. Keyed by range start.
		std::map<uint64, std::pair<uint64, AllocSyncTag>> owners;   // begin -> {end, tag}

		void reset_spans()
		{
			free_spans.clear();
			owners.clear();

			if (start_region < end_region)
				free_spans.insert(span{ start_region, end_region - 1 });
		}

		// First record that could overlap `begin`.
		auto first_overlapping(uint64 begin)
		{
			auto it = owners.upper_bound(begin);
			if (it != owners.begin())
			{
				auto prev = std::prev(it);
				if (prev->second.first >= begin) return prev;
			}
			return it;
		}

		// Is [begin,end] safe for this requester? Every record it overlaps has to be
		// satisfied — after coalescing a candidate can straddle several. Records from
		// older epochs can never constrain anyone again, so they are dropped on sight.
		bool placement_allowed(uint64 begin, uint64 end, const AllocSyncTag& requester)
		{
			for (auto it = first_overlapping(begin); it != owners.end() && it->first <= end; )
			{
				if (it->second.second.stale_for(requester))
				{
					it = owners.erase(it);
					continue;
				}

				if (!it->second.second.satisfied_by(requester)) return false;
				++it;
			}

			return true;
		}

		// Ownership records only describe memory that is free; drop the part just
		// handed out, keeping the head/tail slivers (same previous owner).
		void drop_owner_records(uint64 begin, uint64 end)
		{
			for (auto it = first_overlapping(begin); it != owners.end() && it->first <= end; )
			{
				uint64       rec_begin = it->first;
				uint64       rec_end   = it->second.first;
				AllocSyncTag tag       = it->second.second;

				it = owners.erase(it);

				if (rec_begin < begin) owners.emplace(rec_begin, std::make_pair(begin - 1, tag));
				if (rec_end   > end)   it = owners.emplace(end + 1, std::make_pair(rec_end, tag)).first, ++it;
			}
		}

	public:
		SyncAwareAllocator(uint64 size = std::numeric_limits<uint64>::max())
			: start_region(0), end_region(size), region_size(size)
		{
			reset_spans();
		}

		SyncAwareAllocator(uint64 start_region, uint64 end_region)
			: start_region(start_region), end_region(end_region), region_size(end_region - start_region)
		{
			reset_spans();
		}

		uint64 get_max_usage() const override { return max_usage; }
		uint64 get_size()      const override { return region_size; }

		// Unconstrained: caller has no ordering requirements (or does not track any).
		std::optional<Handle> TryAllocate(uint64 size, uint64 align = 1) override final
		{
			return TryAllocate(size, align, nullptr);
		}

		std::optional<Handle> TryAllocate(uint64 size, uint64 align, const AllocSyncTag* constraint) override final
		{
			if (size == 0)
				return Handle(MemoryInfo(start_region, 0, reset_id), this);

			for (auto it = free_spans.begin(); it != free_spans.end(); ++it)
			{
				uint64 aligned = Math::roundUp(it->begin, align);
				if (aligned < it->begin || aligned + size - 1 > it->end) continue;

				// Skip memory whose previous owner this requester would race. Moving on
				// is the whole point: it relocates the allocation — to another span, or
				// (via PagedAllocator) another page — instead of forcing a barrier.
				if (constraint && !placement_allowed(aligned, aligned + size - 1, *constraint))
					continue;

				span cur = *it;
				free_spans.erase(it);

				if (aligned > cur.begin)             free_spans.insert(span{ cur.begin, aligned - 1 });
				if (aligned + size - 1 < cur.end)    free_spans.insert(span{ aligned + size, cur.end });

				drop_owner_records(aligned, aligned + size - 1);

				max_usage = std::max(max_usage, aligned + size);
				return Handle(MemoryInfo(aligned, size, reset_id), this);
			}

			return std::nullopt;
		}

		void Free(Handle& handle) override
		{
			// Untagged release: the caller tracks no ownership, so the memory is left
			// unconstrained and anyone may take it.
			release(handle, nullptr);
		}

		void Free(Handle& handle, const AllocSyncTag& tag) override
		{
			release(handle, &tag);
		}

		void Reset() override
		{
			reset_id++;
			max_usage = 0;
			reset_spans();
		}

		bool isEmpty() const override
		{
			if (free_spans.size() != 1) return false;

			auto& s = *free_spans.begin();
			return s.begin == start_region && s.end == end_region - 1;
		}

	private:
		void release(Handle& handle, const AllocSyncTag* tag)
		{
			if (!handle) return;
			if (handle.get_reset_id() != reset_id) return;

			ASSERT(handle.get_owner() == this);
			if (!handle.get_size()) return;

			uint64 begin = handle.get_info().offset;
			uint64 end   = begin + handle.get_size() - 1;

#ifdef DEV
			for (auto& s : free_spans)
				ASSERT(s.end < begin || s.begin > end);   // no double free / overlap
#endif

			// Remember who finished with these bytes, before merging them away.
			if (tag) owners.insert_or_assign(begin, std::make_pair(end, *tag));

			// Plain unconditional coalesce — safety lives in `owners`, so nothing here
			// has to reason about it.
			auto next = free_spans.lower_bound(span{ end + 1, 0 });

			if (next != free_spans.end() && next->begin == end + 1)
			{
				end  = next->end;
				next = free_spans.erase(next);
			}

			if (next != free_spans.begin())
			{
				auto prev = std::prev(next);

				if (prev->end + 1 == begin)
				{
					begin = prev->begin;
					free_spans.erase(prev);
				}
			}

			free_spans.insert(span{ begin, end });
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
