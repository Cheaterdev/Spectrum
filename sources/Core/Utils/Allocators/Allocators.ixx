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

			assert(false);

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

	class LinearAllocator : public Allocator
	{

		const uint64 start_region;
		const uint64 end_region;

	protected:
		uint64 offset;
		const uint64 size;
	public:
		using Handle = AllocatorHanle;

		LinearAllocator(uint64 size = std::numeric_limits<uint64>::max()) :size(size), start_region(0), end_region(size)
		{
			offset = start_region;
		}
		LinearAllocator(uint64 start_region, uint64 end_region) :size(end_region - start_region), start_region(start_region), end_region(end_region)
		{
			offset = start_region;
		}
		uint64 get_max_usage() const override
		{
			return offset;
		}

		bool isEmpty() const override
		{	ASSERT_SINGLETHREAD

			return offset == start_region;
		}
		void Free(Handle& handle)
		{
			//		assert(false);
		}
		uint64 get_size()const override
		{
			return size;
		}
		std::optional<Handle> TryAllocate(uint64 size, uint64 align = 1)override final
		{	ASSERT_SINGLETHREAD

			uint64 my_offset = Math::roundUp(offset, align);

			//assert(offset % align == 0);
			if (my_offset + size > this->end_region)
			{
				return std::nullopt;
			}


			offset = my_offset + size;
			Handle h(MemoryInfo(my_offset, size, 0), this);

			assert(h.get_offset()+size<=this->end_region);
			return h;
		}

		void Reset() override
		{	ASSERT_SINGLETHREAD

			offset = start_region;
		}
	};


	//class LinearFreeAllocator : public Allocator
	//{
	//protected:
	//	virtual void moveBlock(uint64 from, uint64 to) = 0;

	//protected:
	//	uint64 offset;
	//	const uint64 size;

	//	std::list<MemoryInfo> allocs;
	//public:
	//	using Handle = AllocatorHanle;

	//	LinearFreeAllocator(uint64 size = std::numeric_limits<uint64>::max()) :size(size)
	//	{
	//		offset = 0;
	//	}

	//	uint64 get_max_usage() const override
	//	{
	//		return offset;
	//	}

	//	Handle Allocate(uint64 size, uint64 align = 1)
	//	{
	//		auto res = TryAllocate(size, align);

	//		if (res)
	//			return *res;

	//		assert(false);

	//		return Handle();
	//	}

	//	uint64 get_size() const override
	//	{
	//		return size;
	//	}
	//	std::optional<Handle> TryAllocate(uint64 size, uint64 align = 1)override final
	//	{
	//		assert(align == 1);
	//		assert(size == 1);

	//		offset = Math::AlignUp(offset, align);


	//		//alloc_ids.resize(max_usage);
	//		return Handle(MemoryInfo(offset, size,0), this);
	//	}


	//	void Free(Handle& handle)
	//	{
	//		if (!handle)
	//			return;

	//		assert(handle.get_owner() == this);
	//		assert(offset > 0);

	//		if (offset > 1)
	//		{
	//			MemoryInfo& last_mem = allocs.back();
	//			const MemoryInfo& removing_mem = handle.get_info();

	//			moveBlock(last_mem.offset, removing_mem.offset);

	//			last_mem.offset = removing_mem.offset;
	//			//last_mem.aligned_offset = removing_mem.aligned_offset;
	//		}


	//		offset--;
	//	}

	//	void Reset() override
	//	{
	//		offset = 0;
	//	}
	//};


	//template<typename T> concept IsAllocatorType = std::is_base_of_v<Allocator, T>;

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
			//assert(handle.get_size());
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
