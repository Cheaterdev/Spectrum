export module Core:PagedAllocators;


import :Allocators;
import :Utils;
import :Math;

import stl.core;
import stl.threading;


template<class T>
concept isCollectable = requires
{
	T::GarbageCount;
};
export namespace Allocators
{

	template<class Context, class LockPolicy>
	class PagePool;

	template<class HeapPageType>
	struct AllocatorPage;


	template<class HeapPageType>
	struct PagedAllocation
	{


		PagedAllocation() = default;
		PagedAllocation(const AllocatorHanle& handle, std::shared_ptr<AllocatorPage<HeapPageType>> page) :handle(handle), page(page)
		{
			offset = handle.get_offset();
			size = handle.get_size();
		}

		std::shared_ptr<HeapPageType> get_heap() const
		{
			auto p = page.lock();
			if (p) return p->get_heap();

			return nullptr;
		}

		operator bool()const
		{
			return !!handle.get_owner();
		}
		inline UINT64 get_offset() const { return offset; }
		inline UINT64 get_size() const { return size; }

		void Free();

		bool CanFree() const;
		bool operator<(const PagedAllocation& other)const
		{
			if (get_heap() != other.get_heap()) return get_heap() < other.get_heap();

			return get_offset() < other.get_offset();
		}

		bool operator!=(const PagedAllocation& other)const
		{
			if (get_heap() != other.get_heap()) return true;

			return get_offset() != other.get_offset();
		}

		bool operator==(const PagedAllocation& other)const
		{
			if (get_heap() != other.get_heap()) return false;

			return get_offset() == other.get_offset();
		}
		//private:

		AllocatorHanle handle;
		std::weak_ptr<AllocatorPage<HeapPageType>>page;
		uint64 offset = 0;
		uint64 size = 0;
	};

	template<class HeapPageType>
	class PageOwnerInterface
	{
	public:
		virtual PagedAllocation<HeapPageType> alloc(size_t size, size_t alignment, const AllocSyncTag* constraint = nullptr) = 0;
		virtual void free(PagedAllocation<HeapPageType>& handle) = 0;

		virtual bool CanFree() = 0;
	};
	template<class Context>
	class PageSourceInterface
	{
	public:
		using ptr_type = std::shared_ptr<typename Context::HeapPageType>;
		using page_type = std::shared_ptr < AllocatorPage<typename Context::HeapPageType>>;
		virtual page_type AllocatePage(Context::HeapMemoryOptions index, size_t size, PageOwnerInterface<typename Context::HeapPageType>& owner) = 0;
		virtual void Free(Context::HeapMemoryOptions index, page_type page) = 0;
	};

	template<class HeapPageType>
	struct AllocatorPage
	{
		using ptr = std::shared_ptr<AllocatorPage>;
		using HandleType = PagedAllocation<HeapPageType>;
		std::shared_ptr<HeapPageType> heap;
		PageOwnerInterface<HeapPageType>& owner;

		std::unique_ptr<Allocator> allocator;
		AllocatorHanle inner_heap_page_handle;
		uint64 offset = 0;

		AllocatorPage(std::shared_ptr<HeapPageType> heap, PageOwnerInterface<HeapPageType>& owner, AllocatorHanle handle) : owner(owner), heap(heap), inner_heap_page_handle(handle)
		{

			//	ASSERT(allocator->get_size()==handle.get_size());
			offset = inner_heap_page_handle.get_offset();
		}
		virtual ~AllocatorPage() = default;
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
			return allocator->get_size();
		}

		UINT64 get_offset() const
		{
			return offset;
		}

		bool CanFree() const
		{
			return owner.CanFree();
		}
	};
	template<class Context, class AllocationPolicy>
	class PageCache : public PageSourceInterface<Context>
	{
	protected:
		using PageSourceInterface<Context>::ptr_type;
		using PageSourceInterface<Context>::page_type;

		struct page_allocator
		{
			ptr_type heap;
			CommonAllocator allocator;
			uint free_counter = 0;
			page_allocator(ptr_type heap, size_t size) :heap(heap), allocator(size) {}
		};

		using heap_list = std::list< page_allocator>;
		std::map<typename Context::HeapMemoryOptions, heap_list> all_heaps;

		typename AllocationPolicy::LockPolicy::mutex m;

		virtual ptr_type make_heap(Context::HeapMemoryOptions index, size_t size) = 0;
	public:

		void GarbageCollect()
		{

			if constexpr (isCollectable<Context>)
			{
				typename AllocationPolicy::LockPolicy::guard g(m);

				for (auto& list : all_heaps)
				{

					for (auto holder = list.second.begin(); holder != list.second.end();)
					{

						if (holder->allocator.isEmpty())
							holder->free_counter++;
						else
							holder->free_counter = 0;


						if (holder->free_counter >= Context::GarbageCount)
						{
							holder = list.second.erase(holder); // reseat iterator to a valid value post-erase
						}
						else {
							++holder;
						}
					}


				}
			}
		}

		page_type AllocatePage(Context::HeapMemoryOptions index, size_t size, PageOwnerInterface<typename Context::HeapPageType>& owner) override
		{
			typename AllocationPolicy::LockPolicy::guard g(m);

			auto& list = all_heaps[index];


			for (auto& holder : list)
			{
				auto handle = holder.allocator.TryAllocate(size, Context::PageAlignment);

				if (handle)
				{
					return  std::make_shared<AllocatorPage<typename Context::HeapPageType>>(holder.heap, owner, *handle);
				}
			}

			auto heap = make_heap(index, size);
			list.emplace_back(heap, heap->get_size());

			auto& holder = list.back();

			auto handle = holder.allocator.Allocate(size, Context::PageAlignment);
			return std::make_shared<AllocatorPage<typename Context::HeapPageType>>(holder.heap, owner, handle);

		}

		void Free(Context::HeapMemoryOptions index, page_type page) override
		{
			typename AllocationPolicy::LockPolicy::guard g(m);
			page->inner_heap_page_handle.FreeAndClear();
			//	free_heaps[index].push_back(page);
		}
	};


	template<class Context, class AllocationPolicy>
	class PagePool :public PageOwnerInterface<typename Context::HeapPageType>
	{
		const typename Context::HeapMemoryOptions creation_info;
		using page_list = std::set<typename  AllocatorPage<typename Context::HeapPageType>::ptr>;


		typename AllocationPolicy::LockPolicy::mutex m;
		page_list all_pages;

		bool del_heaps;



		PageSourceInterface<Context>& factory;
	public:
		PagePool(PageSourceInterface<Context>& factory, Context::HeapMemoryOptions index, bool del_heaps = true) :factory(factory), creation_info(index), del_heaps(del_heaps)
		{

		}


		PagedAllocation<typename Context::HeapPageType> alloc(size_t size, size_t alignment, const AllocSyncTag* constraint = nullptr) override
		{
			typename AllocationPolicy::LockPolicy::guard g(m);


			for (auto& page : all_pages)
			{
				auto handle = page->allocator->TryAllocate(size, alignment, constraint);

				if (handle)
				{

					ASSERT(handle->get_offset() + size <= page->get_offset() + page->get_size());
					return { *handle, page };
				}
			}
			auto pagesize = std::max(Math::roundUp(size, Context::PageAlignment), Context::PageAlignment);


			auto page = factory.AllocatePage(creation_info, pagesize, *this);
			page->allocator.reset(new typename AllocationPolicy::AllocatorType(page->inner_heap_page_handle.get_offset(), page->inner_heap_page_handle.get_offset() + pagesize));
			all_pages.insert(page);
			auto handle = page->allocator->Allocate(size, alignment);
			ASSERT(handle.get_offset() + size <= page->get_offset() + page->get_size());
			return { handle, page };

		}

		void free(PagedAllocation<typename Context::HeapPageType>& handle) override
		{
			typename AllocationPolicy::LockPolicy::guard g(m);

			auto page = handle.page.lock();
			handle.handle.FreeAndClear();

			//uint t = heap.use_count();
			if (del_heaps && page->allocator->isEmpty())
			{
				//				ASSERT(all_pages.count(page) == 1);
				all_pages.erase(page);
				factory.Free(creation_info, page);
			}
		}

		bool CanFree()override
		{
			return  !(std::is_same_v<typename AllocationPolicy::AllocatorType, LinearAllocator>);
		}
		void for_each(std::function<void(const typename Context::HeapMemoryOptions& options, uint64, uint64, std::shared_ptr<typename Context::HeapPageType>)> f)
		{
			typename AllocationPolicy::LockPolicy::guard g(m);
			for (auto& heap : all_pages)
			{
				f(creation_info, heap->get_offset(), (uint64)heap->allocator->get_max_usage(), heap->heap);
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
	class PagedAllocator
	{
		using flags_map = std::map<typename Context::HeapMemoryOptions, std::shared_ptr<PagePool<Context, AllocationPolicy>>>;

		typename AllocationPolicy::LockPolicy::mutex m;
		flags_map creators;
		bool del_heaps;
		PageSourceInterface<Context>& factory;
	public:
		using HeapMemoryOptions = typename Context::HeapMemoryOptions;
		using HandleType = PagedAllocation<typename Context::HeapPageType>;
		PagedAllocator(PageSourceInterface<Context>& factory, bool del_heaps = true) :factory(factory), del_heaps(del_heaps)
		{

		}
		PagedAllocation<typename Context::HeapPageType> alloc(size_t size, size_t alignment, typename Context::HeapMemoryOptions options, const AllocSyncTag* constraint = nullptr)
		{
			ASSERT(size > 0);
			typename AllocationPolicy::LockPolicy::guard g(m);
			auto& creator = creators[options];

			if (!creator)
			{
				creator = std::make_shared<PagePool<Context, AllocationPolicy>>(factory, options, del_heaps);
			}

			auto res = creator->alloc(size, alignment, constraint);
			ASSERT(res.get_offset() % alignment == 0);

			ASSERT(res.get_offset() + size <= res.get_heap()->get_size());
			return res;
		}

		void for_each(std::function<void(const typename Context::HeapMemoryOptions& options, uint64, uint64, std::shared_ptr<typename Context::HeapPageType>)>f)
		{
			typename AllocationPolicy::LockPolicy::guard g(m);
			for (auto& [opt, creator] : creators)
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
	void PagedAllocation<HeapPageType>::Free()
	{
		if (!handle) return;

		auto p = page.lock();
		if (p)
			p->handle_free(*this);
	}


	template<class HeapPageType>
	bool PagedAllocation<HeapPageType>::CanFree() const
	{
		auto p = page.lock();
		if (p)
			return p->CanFree();

		return false;
	}
}

