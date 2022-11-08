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
		size_t offset;
		size_t size;
		size_t aligned_offset;
		size_t reset_id;
		bool operator==(const MemoryInfo& h)const = default;
		bool operator!=(const MemoryInfo& h)const = default;
		MemoryInfo() = default;
		MemoryInfo(const MemoryInfo& r) = default;
	};

	class MemoryInfoProvider
	{
	public:
		virtual const MemoryInfo& get_info() const = 0;
	};


	class DirectMemoryInfoProvider :public MemoryInfoProvider
	{
		MemoryInfo info;
	public:
		DirectMemoryInfoProvider(MemoryInfo& info) :info(info)
		{

		}
		const MemoryInfo& get_info() const override final
		{
			return info;
		}
	};

	class Allocator;

	class AllocatorHanle
	{
	protected:
		std::shared_ptr<MemoryInfoProvider> provider;
		Allocator* owner = nullptr;
	public:
		AllocatorHanle() = default;
		AllocatorHanle(std::shared_ptr<MemoryInfoProvider> provider, Allocator* owner);

		size_t get_offset() const;

		const MemoryInfo& get_info() const
		{
			return provider->get_info();
		}
		size_t get_size();

		operator bool();

		size_t get_reset_id();

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
			AllocatorHanle Allocate(size_t size, size_t align = 1)
		{
			auto res = TryAllocate(size, align);

			if (res)
				return *res;

			assert(false);

			return AllocatorHanle();
		}

		virtual std::optional<AllocatorHanle>  TryAllocate(size_t size, size_t align = 1) = 0;

		virtual void Free(AllocatorHanle& handle) = 0;
	};



	template<class T>
	class AllocationTracker
	{
		std::vector<T> handles;
		std::queue<size_t> freeIds;

	public:
		using Handle = size_t;
		AllocationTracker()
		{
			handles.reserve(256);
		}

		T& place_handle(Handle& res)
		{
			res = handles.size();

			if (freeIds.size())
			{
				res = freeIds.back();
				freeIds.pop();
			}
			else
				handles.emplace_back();

			//	handles[res] = handle;
			return handles[res];
		}

		const T& get_allocation(Handle id) const
		{

			assert(id >= 0 && id < handles.size());
			return handles[id];
		}

		T& get_allocation(Handle id)
		{

			assert(id >= 0 && id < handles.size());
			return handles[id];
		}

		void free(Handle id)
		{
			freeIds.push(id);
		}
	};



	class TrackedMemoryInfoProvider :public MemoryInfoProvider
	{
		size_t id;
		AllocationTracker< MemoryInfo>& tracker;
	public:
		TrackedMemoryInfoProvider(size_t id, AllocationTracker< MemoryInfo>& tracker) :id(id), tracker(tracker)
		{

		}

		const MemoryInfo& get_info() const override final
		{
			return tracker.get_allocation(id);
		}
	};



	//shitty allocator, write a good one
	// dont want to write now

	class CommonAllocator :public Allocator
	{

		size_t size;
		size_t max_usage;
		size_t reset_id;
		struct block
		{
			size_t begin;
			size_t end;

			bool operator< (const block& b) const
			{
				return begin < b.begin;
			}
		};

		std::mutex m;
		std::set<block> free_blocks;
		std::map<size_t, const block*> fences;
		void check();

		size_t merge_prev(size_t start);
		size_t merge_next(size_t end);
		AllocationTracker<MemoryInfo> tracker;


	public:
		using Handle = AllocatorHanle;
		using MemoryHandle = AllocationTracker<MemoryInfo>::Handle;

		CommonAllocator(size_t size = std::numeric_limits<size_t>::max());
		virtual ~CommonAllocator() = default;
		size_t get_max_usage();

		std::optional<Handle> TryAllocate(size_t size, size_t align = 1) override final;
		void Free(Handle& handle);

		void Reset();

		size_t get_size()
		{
			return size;
		}

		bool is_empty()
		{
			ASSERT_SINGLETHREAD


				if (free_blocks.size() == 1)
				{
					auto& b = *free_blocks.begin();
					return (b.begin == 0) && (b.end == size - 1);

				}

			return false;
		}
	};

	class LinearAllocator : public Allocator
	{
	protected:
		size_t offset;
		size_t size;
	public:
		using Handle = AllocatorHanle;
		using MemoryHandle = MemoryInfo;

		LinearAllocator(size_t size = std::numeric_limits<size_t>::max()) :size(size)
		{
			offset = 0;
		}

		size_t get_max_usage()
		{
			return offset;
		}


		void Free(Handle& handle)
		{
			//		assert(false);
		}

		std::optional<Handle> TryAllocate(size_t size, size_t align = 1)override final
		{

			offset = Math::AlignUp(offset, align);

			MemoryInfo info;
			info.aligned_offset = offset;
			info.offset = offset;
			info.size = size;

			offset += size;
			return Handle(std::make_shared<DirectMemoryInfoProvider>(info), this);
		}

		void Reset()
		{
			offset = 0;
		}
	};


	class LinearFreeAllocator : public Allocator
	{
	protected:
		virtual void moveBlock(size_t from, size_t to) = 0;

	protected:
		size_t offset;
		size_t size;
		AllocationTracker<MemoryInfo> tracker;

		std::vector<size_t> alloc_ids;
	public:
		using Handle = AllocatorHanle;
		using MemoryHandle = AllocationTracker<MemoryInfo>::Handle;

		LinearFreeAllocator(size_t size = std::numeric_limits<size_t>::max()) :size(size)
		{
			offset = 0;
		}

		size_t get_max_usage()
		{
			return offset;
		}

		Handle Allocate(size_t size, size_t align = 1)
		{
			auto res = TryAllocate(size, align);

			if (res)
				return *res;

			assert(false);

			return Handle();
		}


		std::optional<Handle> TryAllocate(size_t size, size_t align = 1)override final
		{
			assert(align == 1);
			assert(size == 1);

			offset = Math::AlignUp(offset, align);

			size_t id;
			MemoryInfo& info = tracker.place_handle(id);


			info.aligned_offset = offset;
			info.offset = offset;
			info.size = size;

			alloc_ids.resize(offset + size);
			alloc_ids[offset] = id;

			offset += size;

			//alloc_ids.resize(max_usage);
			return Handle(std::make_shared<TrackedMemoryInfoProvider>(id, tracker), this);
		}


		void Free(Handle& handle)
		{
			if (!handle)
				return;

			assert(handle.get_owner() == this);
			assert(offset > 0);

			if (offset > 1)
			{
				assert(&handle.get_info() == &tracker.get_allocation(alloc_ids[handle.get_offset()]));
				MemoryInfo& last_mem = tracker.get_allocation(alloc_ids[offset - 1]);
				const MemoryInfo& removing_mem = handle.get_info();

				moveBlock(last_mem.offset, removing_mem.offset);

				last_mem.offset = removing_mem.offset;
				last_mem.aligned_offset = removing_mem.aligned_offset;
			}


			offset--;
		}


	};


	//template<typename T> concept IsAllocatorType = std::is_base_of_v<Allocator, T>;

	template<class T> class TypedHandle;
	template<class T>
	class DataProvider
	{
		friend class TypedHandle<T>;
		virtual void Free(TypedHandle<T>& handle) = 0;
	public:
		virtual	T& aquire(size_t offset) {
			return aquire(offset, 1)[0];
		}

		virtual	std::span<T> aquire(size_t offset, size_t size) = 0;

		virtual	void write(size_t offset, std::vector<T>& v) = 0;
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
			//assert(handle.get_size());
		}
		operator bool()
		{
			return provider;
		}

		size_t get_offset()
		{
			return handle.get_offset();
		}

		size_t get_size()
		{
			return handle.get_size();
		}

		std::span<T> aquire()
		{
			return provider->aquire(handle.get_offset(), handle.get_size());
		}

		std::span<T> aquire_one(size_t offset)
		{
			return provider->aquire(handle.get_offset() + offset, 1);
		}

		std::span<T> aquire(size_t size)
		{
			return provider->aquire(handle.get_offset(), size);
		}

		void write(size_t offset, std::vector<T>& r)
		{
			return provider->write(handle.get_offset() + offset, r);
		}

		std::vector<T> map()
		{
			std::vector<T> t;
			t.resize(handle.get_size());
			return t;
		}

		std::vector<T> map(size_t offset)
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

		DataAllocator(size_t count) :AllocatorType(count)
		{

		}

		Handle Allocate(size_t count)
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

		size_t get_max_usage()
		{
			return AllocatorType::get_max_usage();
		}

		virtual void moveBlock(size_t from, size_t to) {

		}

		virtual void on_free(size_t from, size_t to) {

		}

	};
}


CommonAllocator::CommonAllocator(size_t size /*= std::numeric_limits<size_t>::max()*/) :size(size)
{
	reset_id = 0;
	Reset();
}

size_t CommonAllocator::get_max_usage()
{
	return max_usage;
}

std::optional<CommonAllocator::Handle>  CommonAllocator::TryAllocate(size_t size, size_t align)
{
	ASSERT_SINGLETHREAD
		//std::lock_guard<std::mutex> g(m);
		if (size == 0)
		{
			size_t id;
			MemoryInfo& res = tracker.place_handle(id);
			res.aligned_offset = 0;
			res.offset = 0;
			res.size = 0;
			//	res.owner = this;
			res.reset_id = reset_id;
			return Handle(std::make_shared<TrackedMemoryInfoProvider>(id, tracker), nullptr);
		}

	for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
	{
		auto& free_block = *it;
		auto orig_block = free_block;
		auto aligned_offset = Math::roundUp(free_block.begin, align);

		if (free_block.end > aligned_offset)
			if ((free_block.end + 1 - aligned_offset) >= size)
			{

				block used_block;

				used_block.begin = free_block.begin;
				used_block.end = used_block.begin + size + (aligned_offset - free_block.begin) - 1;


				auto free_begin = fences.find(free_block.begin);
				assert(free_begin != fences.end());
				fences.erase(free_begin);
				if (free_block.end != free_block.begin)
				{
					auto free_end = fences.find(free_block.end);
					assert(free_end != fences.end());
					fences.erase(free_end);
				}

				block new_block = free_block;
				free_blocks.erase(it);

				check();


				new_block.begin = used_block.end + 1;
				if (new_block.begin <= new_block.end) {
					auto [new_free, inserted] = free_blocks.insert(new_block);
					fences[new_block.begin] = &*new_free;
					fences[new_block.end] = &*new_free;
				}


				/*

				free_block.begin = used_block.end + 1;

				if (free_block.begin < free_block.end)
				{
					fences[free_block.begin] = &free_block;
				}
				else 	if (free_block.begin > free_block.end)
				{
					auto free_end = fences.find(free_block.end);
					assert(free_end != fences.end());
					fences.erase(free_end);


				}
				*/
				check();

				size_t id;
				MemoryInfo& res = tracker.place_handle(id);

				res.aligned_offset = aligned_offset;
				res.offset = used_block.begin;
				res.size = size;
				//	res.owner = this;
				res.reset_id = reset_id;
				assert(used_block.end < this->size);

				max_usage = std::max(max_usage, used_block.end + 1);

				if (max_usage < this->size / 2)
					assert(!free_blocks.empty());
				return Handle(std::make_shared<TrackedMemoryInfoProvider>(id, tracker), this);
			}

	}
	return std::nullopt;
}
size_t CommonAllocator::merge_prev(size_t start)
{
	if (start == 0) return 0;


	auto prev_free = fences.find(start - 1);

	if (prev_free != fences.end())
	{
		auto& prev_block = prev_free->second;

		size_t result = prev_block->begin;


		auto it = std::find_if(free_blocks.begin(), free_blocks.end(), [&](const block& b) {
			return		b.begin == prev_block->begin;
			});

		free_blocks.erase(it);


		fences.erase(prev_free);

		return result;
	}

	return start;
}
size_t CommonAllocator::merge_next(size_t end)
{

	if (end == size) return end;

	auto next_free = fences.find(end + 1);

	if (next_free != fences.end())
	{
		auto& next_block = next_free->second;

		size_t result = next_block->end;


		auto it = std::find_if(free_blocks.begin(), free_blocks.end(), [&](const block& b) {
			return		b.begin == next_block->begin;
			});

		free_blocks.erase(it);
		fences.erase(next_free);

		return result;
	}

	return end;
}

void CommonAllocator::Free(Handle& handle)
{
	ASSERT_SINGLETHREAD
		//std::lock_guard<std::mutex> g(m);
		if (!handle)
			return;

	if (handle.get_reset_id() != reset_id) return;

	assert(handle.get_owner() == this);
	assert(handle.get_reset_id() == reset_id);



	block my_block;

	my_block.begin = handle.get_info().offset;
	my_block.end = handle.get_info().aligned_offset + handle.get_size() - 1;

	my_block.begin = merge_prev(my_block.begin);
	my_block.end = merge_next(my_block.end);


	assert(my_block.end >= my_block.begin);
	{
		auto [elem, inserted] = free_blocks.insert(my_block);
		auto* block_ptr = &*elem;



		assert(block_ptr->end >= block_ptr->begin);
		fences[my_block.begin] = block_ptr;
		fences[my_block.end] = block_ptr;
	}
	//tracker.free(handle->get_provider)
	//	handle.owner = nullptr;
	check();

}
void  CommonAllocator::check()
{
	//	return;
	for (auto& f : fences)
	{
		bool found = false;

		for (auto& free : free_blocks)
		{
			if (&free == &*f.second)
			{
				found = true;
				break;
			}
		}

		assert(found);


		assert(f.first == f.second->begin || f.first == f.second->end);
	}


	for (auto& f : free_blocks)
	{
		assert(fences[f.begin] == &f);
		assert(fences[f.end] == &f);
	}

}
void CommonAllocator::Reset()
{
	ASSERT_SINGLETHREAD
		free_blocks.clear();
	fences.clear();
	auto [elem, inserted] = free_blocks.insert(block{ 0,size - 1 });
	fences[0] = &*elem;
	fences[size - 1] = &*elem;

	max_usage = 0;
	reset_id++;
	check();

}

bool AllocatorHanle::operator!=(const AllocatorHanle& h) const
{
	return owner != h.owner || (provider->get_info().aligned_offset != h.provider->get_info().aligned_offset);
}

bool AllocatorHanle::operator==(const AllocatorHanle& h) const
{
	return owner == h.owner && (provider->get_info().aligned_offset == h.provider->get_info().aligned_offset);
}

AllocatorHanle::AllocatorHanle(std::shared_ptr<MemoryInfoProvider> provider, Allocator* owner) :provider(provider), owner(owner)
{
	assert(provider);
}

size_t AllocatorHanle::get_offset() const
{
	return provider->get_info().aligned_offset;
}

size_t AllocatorHanle::get_size()
{
	return provider->get_info().size;
}

AllocatorHanle::operator bool()
{
	return !!owner;
}

size_t AllocatorHanle::get_reset_id()
{
	return provider->get_info().reset_id;
}

Allocator* AllocatorHanle::get_owner() const
{
	return owner;
}

void AllocatorHanle::Free()
{
	if (owner) owner->Free(*this);
}
void AllocatorHanle::FreeAndClear()
{
	if (owner) owner->Free(*this);
	owner = nullptr;
}
