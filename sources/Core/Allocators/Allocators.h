#pragma once

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
	virtual MemoryInfo& get_info() = 0;
};


class DirectMemoryInfoProvider :public MemoryInfoProvider
{
	MemoryInfo info;
public:
	DirectMemoryInfoProvider(MemoryInfo& info) :info(info)
	{

	}
	MemoryInfo& get_info() override final
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
	AllocatorHanle();
	AllocatorHanle(std::shared_ptr<MemoryInfoProvider> provider, Allocator* owner);

	size_t get_offset();

	MemoryInfo& get_info()
	{
		return provider->get_info();
	}
	size_t get_size();

	operator bool();

	size_t get_reset_id();

	Allocator* get_owner();

	bool operator==(const AllocatorHanle& h) const;

	bool operator!=(const AllocatorHanle& h) const;

	void Free();
};



class Allocator
{
public:

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

	T& get_allocation(Handle id)
	{
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

	MemoryInfo& get_info() override final
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
		
		alloc_ids.resize(offset+size);
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
			MemoryInfo &last_mem = tracker.get_allocation(alloc_ids[offset - 1]);
			MemoryInfo& removing_mem = handle.get_info();

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

	virtual	void write(size_t offset, std::vector<T> &v) = 0;
};





template<class T>
class TypedHandle
{
	//: public AllocatorHanle

	DataProvider<T>* provider = nullptr;
public:

	AllocatorHanle handle;

	TypedHandle() = default;
	TypedHandle(AllocatorHanle& handle, DataProvider<T>* provider) :handle(handle), provider(provider)
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
	std::span<T> aquire()
	{
		return provider->aquire(handle.get_offset(), handle.get_size());
	}

	std::span<T> aquire_one(size_t offset)
	{
		return provider->aquire(handle.get_offset()+ offset, 1);
	}

	std::span<T> aquire(size_t size)
	{
		return provider->aquire(handle.get_offset(), size);
	}

	void write(size_t offset, std::vector<T>& r)
	{
		return provider->write(handle.get_offset()+offset, r);
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
		if(provider) provider->Free(*this);
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

	void Free(TypedHandle<T>& handle)
	{
		std::lock_guard<std::mutex> g(m);
		handle.handle.Free();
	}

	size_t get_max_usage()
	{
		return AllocatorType::get_max_usage();
	}

	virtual void moveBlock(size_t from, size_t to) {

	}



};
