export module Core:PagedAllocators;


import :Allocators;
import :Utils;
import :Math;
import stl.core;
import stl.threading;



export namespace Allocators
{
	/*class AllocatorContext
	{
		using HeapPageType = HAL::Heap;
		using HeapMemoryOptions = HeapIndex;
		using LockPolicy = Thread::Lockable;

		static const size_t PageAlignment = 4 * 1024 * 1024;
	};*/
	template<class Context, class LockPolicy>
	class HeapAllocator;

	

	


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
		HeapHandle(const CommonAllocator::Handle& handle) :handle(handle)
		{
			offset = handle.get_offset();
		}
		/*Handle(size_t offset, HeapPage<Context>::ptr heap) : heap(heap), offset(offset)
		{
		}*/

		std::shared_ptr<HeapPageType> get_heap() const
		{
			return static_cast<HeapHelper*>(handle.get_owner())->get_heap();
		}
		

		UINT64 get_offset() const
		{
			return offset;
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


	//private:
	//	friend class HeapAllocator<Context>;
		/*HeapAllocatorInterface* owner = nullptr;*/

		CommonAllocator::Handle handle;

		size_t offset;

	};
	template<class Context>
	class HeapFactoryInterface
	{
	public:
		using ptr_type = std::shared_ptr<typename Context::HeapPageType>;
		virtual ptr_type AllocateHeap(Context::HeapMemoryOptions index, size_t size) = 0;
		virtual void Free(Context::HeapMemoryOptions index, ptr_type page) = 0;
	};

	template<class Context>
	class HeapAllocatorInterface
	{
	public:
		virtual HeapHandle<typename Context::HeapPageType> alloc(size_t size, size_t alignment) = 0;
		virtual void free(HeapHandle<typename Context::HeapPageType>& handle) = 0;
	};

	template<class Context, class LockPolicy>
	class HeapFactory: public HeapFactoryInterface<Context>
	{
	protected:
		using HeapFactoryInterface<Context>::ptr_type;
		using heap_list = std::list< ptr_type>;
		std::map<typename Context::HeapMemoryOptions, heap_list> free_heaps;

		typename LockPolicy::mutex m;

		virtual ptr_type make_heap(Context::HeapMemoryOptions index, size_t size) = 0;
	public:
		ptr_type AllocateHeap(Context::HeapMemoryOptions index, size_t size) override
		{
			typename LockPolicy::guard g(m);

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
			typename LockPolicy::guard g(m);
			free_heaps[index].push_back(page);
		}
	};

	template<class Context>
	struct HeapPage : public HeapHandle<typename Context::HeapPageType>::HeapHelper
	{
		using ptr = std::shared_ptr<HeapPage>;
		using HandleType = HeapHandle<typename Context::HeapPageType>;
		std::shared_ptr<typename Context::HeapPageType> heap;
		HeapAllocatorInterface<Context>& owner;
		HeapPage(std::shared_ptr<typename Context::HeapPageType> heap, size_t size, HeapAllocatorInterface<Context>& owner) : HeapHandle<typename Context::HeapPageType>::HeapHelper(size), owner(owner), heap(heap)
		{
		

		}
		virtual ~HeapPage() = default;
		void handle_free(HandleType& handle)
		{
			owner.free(handle);
		}

		std::shared_ptr<typename Context::HeapPageType> get_heap() override
		{
			return heap;
		}

	
	};

	template<class Context, class LockPolicy>
	class HeapAllocator:public HeapAllocatorInterface<Context>
	{
		const typename Context::HeapMemoryOptions creation_info;
		using heap_list = std::list<typename  HeapPage<Context>::ptr>;


		typename LockPolicy::mutex m;
		heap_list all_heaps;
	
		bool del_heaps;



		HeapFactoryInterface<Context>& factory;
	public:
		HeapAllocator(HeapFactoryInterface<Context>& factory,Context::HeapMemoryOptions index, bool del_heaps = true) :factory(factory),creation_info(index), del_heaps(del_heaps)
		{

		}


		HeapHandle<typename Context::HeapPageType> alloc(size_t size, size_t alignment) override
		{
			typename LockPolicy::guard g(m);


			for (auto& heap : all_heaps)
			{
				auto handle = heap->TryAllocate(size, alignment);

				if (handle)
				{
					return { *handle};
				}
			}
			auto pagesize = std::max(Math::AlignUp(size, Context::PageAlignment), Context::PageAlignment);

			auto heap = std::make_shared<HeapPage<Context>>(factory.AllocateHeap(creation_info, pagesize), pagesize, *this);

			all_heaps.emplace_back(heap);
			auto handle = heap->TryAllocate(size, alignment);
			assert(handle);
			return { *handle };

		}

		void free(HeapHandle<typename Context::HeapPageType>& handle) override
		{
			typename LockPolicy::guard g(m);

			auto heap = static_cast<HeapPage<Context>*>(handle.handle.get_owner());
			handle.handle.FreeAndClear();

			//uint t = heap.use_count();
			if (del_heaps && heap->is_empty())
			{
				auto h = std::find_if(all_heaps.begin(), all_heaps.end(), [&](const typename HeapPage<Context>::ptr& p) {return p.get() == heap; });
				auto ptr = *h;

				all_heaps.erase(h);
				factory.Free(creation_info, ptr->heap);
			}
		}

	};



	// for tiles now, only

	template<class Context, class LockPolicy>
	class HeapPageManager
	{
		using flags_map = std::map<typename Context::HeapMemoryOptions, std::shared_ptr<HeapAllocator<Context, LockPolicy>>>;

		typename LockPolicy::mutex m;
		flags_map creators;
		bool del_heaps;
		HeapFactoryInterface<Context>& factory;
	public:
		HeapPageManager(HeapFactoryInterface<Context>&factory, bool del_heaps = true) :factory(factory),del_heaps(del_heaps)
		{

		}
		HeapHandle<typename Context::HeapPageType> alloc(size_t size, size_t alignment, typename Context::HeapMemoryOptions options)
		{
			typename LockPolicy::guard g(m);
			auto& creator = creators[options];

			if (!creator)
			{
				creator = std::make_shared<HeapAllocator<Context, LockPolicy>>(factory,options, del_heaps);
			}

			return creator->alloc(size, alignment);
		}

	};


	template<class HeapPageType>
	void HeapHandle<HeapPageType>::Free()
	{
		if (!handle) return;
		static_cast<HeapHelper*>(handle.get_owner())->handle_free(*this);
	}
}


