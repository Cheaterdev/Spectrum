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
		size_t offset = 0;
		size_t size = 0;
		//	size_t aligned_offset;
		size_t reset_id = 0;
		//	size_t alloc_id;
		bool operator==(const MemoryInfo& h)const = default;
		bool operator!=(const MemoryInfo& h)const = default;
		MemoryInfo() = default;
		MemoryInfo(const MemoryInfo& r) = default;
		MemoryInfo(size_t offset, size_t size,size_t reset_id) :offset(offset), size(size),reset_id(reset_id)
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

		size_t get_offset() const;

		const MemoryInfo& get_info() const
		{
			return info;
		}
		size_t get_size() const;

		operator bool() const;

		size_t get_reset_id()const;

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

		virtual bool isEmpty() const = 0;

		virtual void Reset() = 0;

		virtual size_t get_max_usage() const = 0;
		virtual size_t get_size() const = 0;
	};



	//shitty allocator, write a good one
	// dont want to write now

	class CommonAllocator :public Allocator
	{
		const size_t start_region;
		const size_t end_region;
		const size_t size;
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

	public:
		using Handle = AllocatorHanle;

		CommonAllocator(size_t size = std::numeric_limits<size_t>::max());

		CommonAllocator(size_t start_region, size_t end_region);

		virtual ~CommonAllocator() = default;
		size_t get_max_usage() const override;

		std::optional<Handle> TryAllocate(size_t size, size_t align = 1) override final;
		void Free(Handle& handle);

		void Reset() override;

		size_t get_size() const override
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

	class LinearAllocator : public Allocator
	{

		const size_t start_region;
		const size_t end_region;

	protected:
		size_t offset;
		const size_t size;
	public:
		using Handle = AllocatorHanle;

		LinearAllocator(size_t size = std::numeric_limits<size_t>::max()) :size(size), start_region(0), end_region(size)
		{
			offset = start_region;
		}
		LinearAllocator(size_t start_region, size_t end_region) :size(end_region - start_region), start_region(start_region), end_region(end_region)
		{
			offset = start_region;
		}
		size_t get_max_usage() const override
		{
			return offset;
		}

		bool isEmpty() const override
		{
			return offset == start_region;
		}
		void Free(Handle& handle)
		{
			//		assert(false);
		}
		size_t get_size()const override
		{
			return size;
		}
		std::optional<Handle> TryAllocate(size_t size, size_t align = 1)override final
		{
			size_t my_offset = Math::roundUp(offset, align);

			//assert(offset % align == 0);
			if (my_offset + size > this->end_region)
			{
				return std::nullopt;
			}


			offset = my_offset + size;
			return Handle(MemoryInfo(my_offset, size, 0), this);
		}

		void Reset() override
		{
			offset = start_region;
		}
	};


	class LinearFreeAllocator : public Allocator
	{
	protected:
		virtual void moveBlock(size_t from, size_t to) = 0;

	protected:
		size_t offset;
		const size_t size;

		std::list<MemoryInfo> allocs;
	public:
		using Handle = AllocatorHanle;

		LinearFreeAllocator(size_t size = std::numeric_limits<size_t>::max()) :size(size)
		{
			offset = 0;
		}

		size_t get_max_usage() const override
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

		size_t get_size() const override
		{
			return size;
		}
		std::optional<Handle> TryAllocate(size_t size, size_t align = 1)override final
		{
			assert(align == 1);
			assert(size == 1);

			offset = Math::AlignUp(offset, align);


			//alloc_ids.resize(max_usage);
			return Handle(MemoryInfo(offset, size,0), this);
		}


		void Free(Handle& handle)
		{
			if (!handle)
				return;

			assert(handle.get_owner() == this);
			assert(offset > 0);

			if (offset > 1)
			{
				MemoryInfo& last_mem = allocs.back();
				const MemoryInfo& removing_mem = handle.get_info();

				moveBlock(last_mem.offset, removing_mem.offset);

				last_mem.offset = removing_mem.offset;
				//last_mem.aligned_offset = removing_mem.aligned_offset;
			}


			offset--;
		}

		void Reset() override
		{
			offset = 0;
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

		size_t get_offset() const
		{
			return handle.get_offset();
		}

		size_t get_size() const
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

CommonAllocator::CommonAllocator(size_t start_region, size_t end_region) :size(end_region - start_region), start_region(start_region), end_region(end_region)
{
	reset_id = 0;
	Reset();
}
CommonAllocator::CommonAllocator(size_t size /*= std::numeric_limits<size_t>::max()*/) : size(size), start_region(0), end_region(size)
{
	reset_id = 0;
	Reset();
}

size_t CommonAllocator::get_max_usage() const
{
	return max_usage;
}

std::optional<CommonAllocator::Handle>  CommonAllocator::TryAllocate(size_t size, size_t align)
{
	ASSERT_SINGLETHREAD
		//std::lock_guard<std::mutex> g(m);
		if (size == 0)
		{
			//size_t id;
			//MemoryInfo& res = tracker.place_handle(id);
			//res.aligned_offset = start_region;
			//res.offset = start_region;
			//res.size = start_region;
			////	res.owner = this;
			//res.reset_id = reset_id;
			return Handle(MemoryInfo(start_region, 0,0), this);
		}


	auto make_free_block = [this](block& new_block)
	{
		if (new_block.begin <= new_block.end) {
			auto [new_free, inserted] = free_blocks.insert(new_block);
			fences[new_block.begin] = &*new_free;
			fences[new_block.end] = &*new_free;
		}

	};


	for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it)
	{
		auto& free_block = *it;
		block orig_block = free_block;
		auto aligned_offset = Math::roundUp(free_block.begin, align);

		if (free_block.end > aligned_offset)
			if ( aligned_offset + size - 1 <= free_block.end)
			{

				block used_block;

				used_block.begin = aligned_offset;
				used_block.end = aligned_offset + size - 1;


				auto free_begin = fences.find(free_block.begin);
				assert(free_begin != fences.end());
				fences.erase(free_begin);
				if (free_block.end != free_block.begin)
				{
					auto free_end = fences.find(free_block.end);
					assert(free_end != fences.end());
					fences.erase(free_end);
				}
				free_blocks.erase(it);

				check();

				if (aligned_offset > used_block.begin)
				{
					auto prev_free = orig_block;
					prev_free.end = used_block.begin - 1;
					make_free_block(prev_free);
				}

				auto next_free = orig_block;

				next_free.begin = used_block.end + 1;
				make_free_block(next_free);


				check();

				assert(used_block.end < end_region);

				max_usage = std::max(max_usage, used_block.end + 1);

				/*	if (max_usage < this->size / 2)
						assert(!free_blocks.empty());*/
				return Handle(MemoryInfo(aligned_offset, size, reset_id), this);
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
	my_block.end = handle.get_info().offset + handle.get_size() - 1;

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
#ifdef DEV
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

#endif
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
	auto [elem, inserted] = free_blocks.insert(block{ start_region,end_region - 1 });
	fences[start_region] = &*elem;
	fences[end_region - 1] = &*elem;

	max_usage = 0;
	reset_id++;
	check();

}

bool AllocatorHanle::operator!=(const AllocatorHanle& h) const
{
	return owner != h.owner || info != h.info;
}

bool AllocatorHanle::operator==(const AllocatorHanle& h) const
{
	return owner == h.owner && info == h.info;
}

AllocatorHanle::AllocatorHanle(const MemoryInfo& info, Allocator* owner) :info(info), owner(owner)
{
	//	assert(provider);
}

size_t AllocatorHanle::get_offset() const
{
	return info.offset;
}

size_t AllocatorHanle::get_size() const
{
	return info.size;
}

AllocatorHanle::operator bool() const
{
	return !!owner;
}

size_t AllocatorHanle::get_reset_id() const
{
	return  info.reset_id;
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
