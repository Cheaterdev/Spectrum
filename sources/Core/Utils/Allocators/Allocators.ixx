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
		template<class T = std::byte>
			AllocatorHanle Allocate(size_t size, size_t align = sizeof(T))
		{
			auto res = TryAllocate(size*sizeof(T), align);

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

			size_t get_offset_in_bytes() const
		{
			return handle.get_offset()*sizeof(T);
		}

		size_t get_size_in_bytes() const
		{
			return handle.get_size()*sizeof(T);
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
