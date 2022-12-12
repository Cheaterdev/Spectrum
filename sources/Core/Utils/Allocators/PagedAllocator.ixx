export module Core:PagedAllocators;


import :Allocators;
import :Utils;
import :Math;
import stl.core;
import stl.threading;



export namespace Allocators
{

	template<class Context, class LockPolicy>
	class HeapAllocator;

	template<class HeapPageType>
	struct HeapPage;


	template<class HeapPageType>
	struct HeapHandle
	{
		

		HeapHandle() = default;
		HeapHandle(const AllocatorHanle& handle, std::shared_ptr<HeapPage<HeapPageType>> page) :handle(handle), page(page)
		{
		offset=handle.get_offset() + page->get_offset();
			size=handle.get_size();
	}

		std::shared_ptr<HeapPageType> get_heap() const
		{
			auto p =page.lock();
		if(p) return p->get_heap();

			return nullptr;
		}
		
		operator bool()const
		{
			return !!handle.get_owner();
		}
		inline UINT64 get_offset() const{return offset;}
		inline UINT64 get_size() const {return size;}

		void Free();

		bool CanFree() const;
		bool operator<(const HeapHandle& other)const
		{
			if (get_heap() != other.get_heap()) return get_heap() < other.get_heap();

			return get_offset() < other.get_offset();
		}

		bool operator!=(const HeapHandle& other)const
		{
			if (get_heap() != other.get_heap()) return true;

			return get_offset() != other.get_offset();
		}

		bool operator==(const HeapHandle& other)const
		{
			if (get_heap() != other.get_heap()) return false;

			return get_offset() == other.get_offset();
		}
	//private:

		AllocatorHanle handle;
		std::weak_ptr<HeapPage<HeapPageType>>page;
		uint64 offset = 0;
		uint64 size = 0;
	};
	
	template<class HeapPageType>
	class HeapAllocatorInterface
	{
	public:
		virtual HeapHandle<HeapPageType> alloc(size_t size, size_t alignment) = 0;
		virtual void free(HeapHandle<HeapPageType>& handle) = 0;

			virtual bool CanFree() = 0;
	};
	template<class Context>
	class HeapFactoryInterface
	{
	public:
		using ptr_type = std::shared_ptr<typename Context::HeapPageType>;
		using page_type = std::shared_ptr < HeapPage<typename Context::HeapPageType>>;
		virtual page_type AllocateHeap(Context::HeapMemoryOptions index, size_t size, HeapAllocatorInterface<typename Context::HeapPageType>& owner) = 0;
		virtual void Free(Context::HeapMemoryOptions index, page_type page) = 0;
	};

	template<class HeapPageType>
	struct HeapPage
	{
		using ptr = std::shared_ptr<HeapPage>;
		using HandleType = HeapHandle<HeapPageType>;
		std::shared_ptr<HeapPageType> heap;
		HeapAllocatorInterface<HeapPageType>& owner;

		std::unique_ptr<Allocator> allocator;
		AllocatorHanle inner_heap_page_handle;
		uint64 offset = 0;
		
		HeapPage(std::shared_ptr<HeapPageType> heap, HeapAllocatorInterface<HeapPageType>& owner,AllocatorHanle handle) : owner(owner), heap(heap),inner_heap_page_handle(handle)
		{
	
		//	assert(allocator->get_size()==handle.get_size());
			offset=inner_heap_page_handle.get_offset();
		}
		virtual ~HeapPage() = default;
		void handle_free(HandleType& handle) 
		{
			owner.free(handle);
		}

		std::shared_ptr<HeapPageType> get_heap() 
		{
			return heap;
		}

		size_t get_size()
		{
			return allocator.get_size();
		}

		UINT64 get_offset() const
		{
			return 0;
		}

			bool CanFree() const
		{
			return owner.CanFree();
		}
	};
	template<class Context, class AllocationPolicy>
	class HeapFactory: public HeapFactoryInterface<Context>
	{
	protected:
		using HeapFactoryInterface<Context>::ptr_type;
		using HeapFactoryInterface<Context>::page_type;
		
		struct page_allocator
		{
			ptr_type heap;
			CommonAllocator allocator;

			page_allocator(ptr_type heap,size_t size):heap(heap),allocator(size){}
		};
	
		using heap_list = std::list< page_allocator>;
		std::map<typename Context::HeapMemoryOptions, heap_list> all_heaps;

		typename AllocationPolicy::LockPolicy::mutex m;

		virtual ptr_type make_heap(Context::HeapMemoryOptions index, size_t size) = 0;
	public:
		page_type AllocateHeap(Context::HeapMemoryOptions index, size_t size, HeapAllocatorInterface<typename Context::HeapPageType>& owner) override
		{
			typename AllocationPolicy::LockPolicy::guard g(m);

			auto& list = all_heaps[index];


				for (auto &holder:list)
				{
					auto handle = holder.allocator.TryAllocate(size, Context::PageAlignment);

					if (handle)
					{
						return  std::make_shared<HeapPage<typename Context::HeapPageType>>(holder.heap, owner,*handle);
					}
				}
				
			auto heap = make_heap(index, size);
			list.emplace_back(heap, heap->get_size() );

			auto& holder = list.back();

			auto handle = holder.allocator.Allocate(size, Context::PageAlignment);
			return std::make_shared<HeapPage<typename Context::HeapPageType>>(holder.heap,  owner,handle);
		
		}

		void Free(Context::HeapMemoryOptions index, page_type page) override
		{
			typename AllocationPolicy::LockPolicy::guard g(m);
			page->inner_heap_page_handle.FreeAndClear();
		//	free_heaps[index].push_back(page);
		}
	};


	template<class Context, class AllocationPolicy>
	class HeapAllocator:public HeapAllocatorInterface<typename Context::HeapPageType>
	{
		const typename Context::HeapMemoryOptions creation_info;
		using page_list = std::set<typename  HeapPage<typename Context::HeapPageType>::ptr>;


		typename AllocationPolicy::LockPolicy::mutex m;
		page_list all_pages;
	
		bool del_heaps;



		HeapFactoryInterface<Context>& factory;
	public:
		HeapAllocator(HeapFactoryInterface<Context>& factory,Context::HeapMemoryOptions index, bool del_heaps = true) :factory(factory),creation_info(index), del_heaps(del_heaps)
		{

		}


		HeapHandle<typename Context::HeapPageType> alloc(size_t size, size_t alignment) override
		{
			typename AllocationPolicy::LockPolicy::guard g(m);


			for (auto& page : all_pages)
			{
				auto handle = page->allocator->TryAllocate(size, alignment);

				if (handle)
				{
					return { *handle, page};
				}
			}
			auto pagesize = std::max(Math::roundUp(size, Context::PageAlignment), Context::PageAlignment);

		
			auto page = factory.AllocateHeap(creation_info, pagesize,*this);
			page->allocator.reset(new typename AllocationPolicy::AllocatorType(page->inner_heap_page_handle.get_offset(),page->inner_heap_page_handle.get_offset()+pagesize));
			all_pages.insert(page);
			auto handle = page->allocator->Allocate(size, alignment);
			return { handle, page};

		}

		void free(HeapHandle<typename Context::HeapPageType>& handle) override
		{
			typename AllocationPolicy::LockPolicy::guard g(m);

			auto page = handle.page.lock();
			handle.handle.FreeAndClear();

			//uint t = heap.use_count();
			if (del_heaps && page->allocator->isEmpty())
			{
//				assert(all_pages.count(page) == 1);
				all_pages.erase(page);
				factory.Free(creation_info, page);
			}
		}

		 bool CanFree()override
		{
			return  !(std::is_same_v<typename AllocationPolicy::AllocatorType,LinearAllocator>);
		}
		void for_each(std::function<void(const typename Context::HeapMemoryOptions& options, size_t max_usage, std::shared_ptr<typename Context::HeapPageType>)> f)
		{
			typename AllocationPolicy::LockPolicy::guard g(m);
			for (auto &heap : all_pages)
			{
				f(creation_info, heap->allocator->get_max_usage(), heap->heap);
			}
		}

		void reset()
		{
		typename AllocationPolicy::LockPolicy::guard g(m);
	for (auto& page : all_pages)
			{
				page->allocator->Reset();
				factory.Free(creation_info, page);
			}

			all_pages.clear();
		}
	};



	// for tiles now, only

	template<class Context, class AllocationPolicy>
	class HeapPageManager
	{
		using flags_map = std::map<typename Context::HeapMemoryOptions, std::shared_ptr<HeapAllocator<Context, AllocationPolicy>>>;

		typename AllocationPolicy::LockPolicy::mutex m;
		flags_map creators;
		bool del_heaps;
		HeapFactoryInterface<Context>& factory;
	public:
		using HeapMemoryOptions = typename Context::HeapMemoryOptions;
		using HandleType = HeapHandle<typename Context::HeapPageType>;
		HeapPageManager(HeapFactoryInterface<Context>&factory, bool del_heaps = true) :factory(factory),del_heaps(del_heaps)
		{

		}
		HeapHandle<typename Context::HeapPageType> alloc(size_t size, size_t alignment, typename Context::HeapMemoryOptions options)
		{
			assert(size > 0);
			typename AllocationPolicy::LockPolicy::guard g(m);
			auto& creator = creators[options];

			if (!creator)
			{
				creator = std::make_shared<HeapAllocator<Context, AllocationPolicy>>(factory,options, del_heaps);
			}

			auto res= creator->alloc(size, alignment);
			assert(res.get_offset()%alignment == 0);
			return res;
		}

		void for_each(std::function<void(const typename Context::HeapMemoryOptions&options, size_t max_usage, std::shared_ptr<typename Context::HeapPageType>)>f)
		{
			typename AllocationPolicy::LockPolicy::guard g(m);
			for(auto &[opt,creator]:creators)
			{
				creator->for_each(f);
			}
		}

		void reset()
		{
			for (auto& [opt, creator] : creators)
			{
				creator->reset();
			}
		}
	};


	template<class HeapPageType>
	void HeapHandle<HeapPageType>::Free()
	{
		if (!handle) return;

		auto p =page.lock();
		if(p)
		p->handle_free(*this);
	}


	template<class HeapPageType>
	bool HeapHandle<HeapPageType>::CanFree() const
	{	auto p =page.lock();
		if(p)
		return p->CanFree();

			return false;
	}
}


