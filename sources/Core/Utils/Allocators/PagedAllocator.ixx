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

		class HeapHelper: public CommonAllocator {
		public:

			HeapHelper(size_t size):CommonAllocator(size)
			{
				
			}
			virtual void handle_free(HeapHandle<HeapPageType>& handle) = 0;
			virtual std::shared_ptr<HeapPageType> get_heap() = 0;
		};



		HeapHandle() = default;
		HeapHandle(const AllocatorHanle& handle, HeapPage<HeapPageType>* page) :handle(handle), page(page)
		{
		
		}

		std::shared_ptr<HeapPageType> get_heap() const
		{
			return page->get_heap();
		}
		

		UINT64 get_offset() const
		{
			return handle.get_offset();
		}

		void Free();

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
		

		AllocatorHanle handle;
		HeapPage<HeapPageType> *page = nullptr;

	};
	template<class Context>
	class HeapFactoryInterface
	{
	public:
		using ptr_type = std::shared_ptr<typename Context::HeapPageType>;
		virtual ptr_type AllocateHeap(Context::HeapMemoryOptions index, size_t size) = 0;
		virtual void Free(Context::HeapMemoryOptions index, ptr_type page) = 0;
	};

	template<class HeapPageType>
	class HeapAllocatorInterface
	{
	public:
		virtual HeapHandle<HeapPageType> alloc(size_t size, size_t alignment) = 0;
		virtual void free(HeapHandle<HeapPageType>& handle) = 0;
	};

	template<class Context, class AllocationContext>
	class HeapFactory: public HeapFactoryInterface<Context>
	{
	protected:
		using HeapFactoryInterface<Context>::ptr_type;
		using heap_list = std::list< ptr_type>;
		std::map<typename Context::HeapMemoryOptions, heap_list> free_heaps;

		typename AllocationContext::LockPolicy::mutex m;

		virtual ptr_type make_heap(Context::HeapMemoryOptions index, size_t size) = 0;
	public:
		ptr_type AllocateHeap(Context::HeapMemoryOptions index, size_t size) override
		{
			typename AllocationContext::LockPolicy::guard g(m);

			auto& list = free_heaps[index];
			if (!list.empty())
			{
				for (auto it = list.begin(); it != list.end(); ++it)
				{
					if ((*it)->get_size() >= size)
					{
						auto first = *it;
						list.erase(it);
						return first;
					}
				}

			}

			return make_heap(index, size); 
		}

		void Free(Context::HeapMemoryOptions index, ptr_type page) override
		{
			typename AllocationContext::LockPolicy::guard g(m);
			free_heaps[index].push_back(page);
		}
	};

	template<class HeapPageType>
	struct HeapPage
	{
		using ptr = std::shared_ptr<HeapPage>;
		using HandleType = HeapHandle<HeapPageType>;
		std::shared_ptr<HeapPageType> heap;
		HeapAllocatorInterface<HeapPageType>& owner;

		std::unique_ptr<Allocator> allocator;

		HeapPage(std::shared_ptr<HeapPageType> heap,  Allocator* allocator, HeapAllocatorInterface<HeapPageType>& owner) : owner(owner), heap(heap)
		{
			this->allocator.reset(allocator);

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

	
	};

	template<class Context, class AllocationContext>
	class HeapAllocator:public HeapAllocatorInterface<typename Context::HeapPageType>
	{
		const typename Context::HeapMemoryOptions creation_info;
		using page_list = std::list<typename  HeapPage<typename Context::HeapPageType>::ptr>;


		typename AllocationContext::LockPolicy::mutex m;
		page_list all_pages;
	
		bool del_heaps;



		HeapFactoryInterface<Context>& factory;
	public:
		HeapAllocator(HeapFactoryInterface<Context>& factory,Context::HeapMemoryOptions index, bool del_heaps = true) :factory(factory),creation_info(index), del_heaps(del_heaps)
		{

		}


		HeapHandle<typename Context::HeapPageType> alloc(size_t size, size_t alignment) override
		{
			typename AllocationContext::LockPolicy::guard g(m);


			for (auto& page : all_pages)
			{
				auto handle = page->allocator->TryAllocate(size, alignment);

				if (handle)
				{
					return { *handle, page.get()};
				}
			}
			auto pagesize = std::max(Math::roundUp(size, Context::PageAlignment), Context::PageAlignment);

			auto page = std::make_shared<HeapPage<typename Context::HeapPageType>>(factory.AllocateHeap(creation_info, pagesize), new typename AllocationContext::AllocatorType(pagesize), *this);

			all_pages.emplace_back(page);
			auto handle = page->allocator->TryAllocate(size, alignment);
			assert(handle);
			return { *handle, page.get() };

		}

		void free(HeapHandle<typename Context::HeapPageType>& handle) override
		{
			typename AllocationContext::LockPolicy::guard g(m);

			auto page = handle.page;
			handle.handle.FreeAndClear();

			//uint t = heap.use_count();
			if (del_heaps && page->allocator->isEmpty())
			{
				auto h = std::find_if(all_pages.begin(), all_pages.end(), [&](const typename HeapPage<typename Context::HeapPageType>::ptr& p) {return p.get() == page; });
				auto ptr = *h;

				all_pages.erase(h);
				factory.Free(creation_info, ptr->heap);
			}
		}

		void for_each(std::function<void(const typename Context::HeapMemoryOptions& options, size_t max_usage, std::shared_ptr<typename Context::HeapPageType>)> f)
		{
			typename AllocationContext::LockPolicy::guard g(m);
			for (auto &heap : all_pages)
			{
				f(creation_info, heap->allocator->get_max_usage(), heap->heap);
			}
		}

		void reset()
		{
			for (auto& page : all_pages)
			{
				page->allocator->Reset();
				factory.Free(creation_info, page->heap);
			}

			all_pages.clear();
		}
	};



	// for tiles now, only

	template<class Context, class AllocationContext>
	class HeapPageManager
	{
		using flags_map = std::map<typename Context::HeapMemoryOptions, std::shared_ptr<HeapAllocator<Context, AllocationContext>>>;

		typename AllocationContext::LockPolicy::mutex m;
		flags_map creators;
		bool del_heaps;
		HeapFactoryInterface<Context>& factory;
	public:
		HeapPageManager(HeapFactoryInterface<Context>&factory, bool del_heaps = true) :factory(factory),del_heaps(del_heaps)
		{

		}
		HeapHandle<typename Context::HeapPageType> alloc(size_t size, size_t alignment, typename Context::HeapMemoryOptions options)
		{
			typename AllocationContext::LockPolicy::guard g(m);
			auto& creator = creators[options];

			if (!creator)
			{
				creator = std::make_shared<HeapAllocator<Context, AllocationContext>>(factory,options, del_heaps);
			}

			return creator->alloc(size, alignment);
		}

		void for_each(std::function<void(const typename Context::HeapMemoryOptions&options, size_t max_usage, std::shared_ptr<typename Context::HeapPageType>)>f)
		{
			typename AllocationContext::LockPolicy::guard g(m);
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
		page->handle_free(*this);
	}
}


