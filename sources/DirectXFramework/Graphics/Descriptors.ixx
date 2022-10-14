export module Graphics:Descriptors;


import Allocators;
import Math;
import :Resource;

import stl.threading;
import stl.core;

import Log;
import Data;
import Events;
import Threading;
import HAL;
import d3d12;

//import :Definitions;
using namespace HAL;
export
{

	namespace Graphics
	{
		DescriptorHeapType get_heap_type(HandleType type);


		struct ResourceInfo
		{
			Resource* resource_ptr = nullptr;
			Resource* counter_resource = nullptr;

			std::variant<HAL::Views::DepthStencil, HAL::Views::UnorderedAccess, HAL::Views::ShaderResource, HAL::Views::RenderTarget, HAL::Views::ConstantBuffer> view;


			ResourceInfo() = default;

			ResourceInfo(Resource* resource_ptr, HAL::Views::ShaderResource srv) :resource_ptr(resource_ptr)
			{
				view = srv;
			}

			ResourceInfo(Resource* resource_ptr, HAL::Views::DepthStencil dsv) :resource_ptr(resource_ptr)
			{
				view = dsv;
			}

			ResourceInfo(Resource* resource_ptr, Resource* counter_ptr, HAL::Views::UnorderedAccess uav) :resource_ptr(resource_ptr)
			{
				view = uav;
				this->counter_resource = counter_ptr;
			}

			ResourceInfo(Resource* resource_ptr, HAL::Views::RenderTarget rtv) :resource_ptr(resource_ptr)
			{
				view = rtv;
			}

			ResourceInfo(Resource* resource_ptr, HAL::Views::ConstantBuffer cbv) :resource_ptr(resource_ptr)
			{
				view = cbv;
			}

			void for_each_subres(std::function<void(Resource*, UINT)> f) const;
		};

		//class DescriptorHeap;
		struct Handle
		{
			DescriptorHeap* heap = nullptr;

			ResourceInfo* get_resource_info() const;

			bool is_valid() const
			{
				return heap && offset != UINT_MAX;
			}

			operator bool() const;
			void operator=(const std::function<void(Handle&)>& f)
			{
				f(*this);
			}

			UINT offset = UINT_MAX;


			bool operator!=(const Handle& r)
			{
				if (offset != r.offset) return true;
				return false;
			}

			void place(const Handle& r) const;

			template<HAL::Views::ViewTemplate T>
			void operator=(const T& v);



			void operator=(const Handle& r)
			{
				//				assert(r.heap);
				heap = r.heap;
				offset = r.offset;
			}
			D3D12_CPU_DESCRIPTOR_HANDLE get_cpu() const;

			D3D12_GPU_DESCRIPTOR_HANDLE get_gpu()const;
			//	void clear(CommandList& list, float4 = { 0, 0, 0, 0 }) const;
		};


		//	class DescriptorHeap;
			//	class CommandList;
		struct HandleTable
		{
			Handle operator[](UINT i) const
			{
				assert(i < info->count);
				Handle res;

				res.heap = info->heap;
				if (info->offset != UINT_MAX) res.offset = info->offset + i;
				return res;
			}

			/*Handle& get_base() const
			{
				return info->base;
			}
			*/
			UINT get_count() const
			{
				return info ? info->count : 0;
			}

			bool valid() const
			{
				return !!info;
			}


		private:
			friend class DescriptorHeap;

			struct helper
			{
				DescriptorHeap* heap;
				UINT offset;
				UINT count;
			};


			std::shared_ptr<helper> info;
		};

		struct HandleTableLight : public Handle
		{
			Handle operator[](UINT i) const
			{
				assert(i < count);
				Handle res = *this;
				if (offset != UINT_MAX)  res.offset += i;
				return res;
			}


			inline UINT get_count() const
			{
				return count;
			}

			bool valid() const
			{
				return count > 0;
			}


		private:
			friend class DescriptorHeap;
			UINT count = 0;
		};

		class DescriptorHeap : public SharedObject<DescriptorHeap>
		{
			friend class Handle;
			HAL::DescriptorHeap native_heap;
			std::vector<ResourceInfo> resources;
			DescriptorHeapFlags flags;

			UINT descriptor_size;
			UINT current_offset = 0;
			UINT max_count;
			std::mutex m;
			std::map<UINT, std::deque<UINT>> frees;
			UINT used_size = 0;
		public:

			auto get_dx() const
			{
				if (native_heap.m_gpu_heap)
					return native_heap.m_gpu_heap.Get();
				return native_heap.m_cpu_heap.Get();
			}


			Events::Event<void> on_free;
			HandleTable make_table(UINT count, UINT offset)
			{
				HandleTable res;
				std::weak_ptr<DescriptorHeap> ptr = get_ptr();
				res.info = std::shared_ptr<HandleTable::helper>(new HandleTable::helper, [ptr, count, offset](HandleTable::helper* e)
					{
						auto t = ptr.lock();
						if (t)
						{
							std::lock_guard<std::mutex> g(t->m);


							t->frees[count].push_back(offset);

							t->used_size -= e->count;

							if (t->used_size == 0)
							{
								t->on_free();
							}
						}

						delete e;
					});
				used_size += count;
				res.info->heap = this;
				res.info->offset = offset;
				res.info->count = count;
				return res;
			}
			template<class T>
			void place(HandleTable& table, UINT i, T smth)
			{
				smth(table[i]);
			}

			template<class T, class ...Args>
			void place(HandleTable& table, UINT i, T smth, Args...args)
			{
				place(table, i, smth);
				place(table, i + 1, args...);
			}

			friend struct Handle;

			ResourceInfo* get_resource_info(const Handle* h)
			{
				return &resources[h->offset];
			}
		public:
			using ptr = std::shared_ptr<DescriptorHeap>;

			DescriptorHeap(UINT num, DescriptorHeapType type, DescriptorHeapFlags flags = DescriptorHeapFlags::NONE);

			virtual ~DescriptorHeap()
			{

			}
			void reset()
			{
				current_offset = 0;
			}

			size_t size()
			{
				return max_count;
			}
			template<class T>
			HandleTable create_table(T _count) requires(std::is_scalar_v<T>)
			{

				UINT count = static_cast<UINT>(_count);
				std::lock_guard<std::mutex> g(m);

				// TODO: Uncomment
			//	for(auto it = frees.lower_bound(count); it!=frees.end();it++)
			//	if (it->size())
				if (frees[count].size())
				{
					auto&& list = frees[count];

					UINT offset = list.front();
					list.pop_front();
					/*
										if (it->size() > _count)
										{
											auto delta = it->size() - _count;
											frees[delta].emplace_back(offset+_count);
										}*/
					return make_table(count, offset);
				}

				if (current_offset >= max_count)
					Log::get() << Log::LEVEL_ERROR << "DescriptorHeap limit reached!" << Log::endl;

				HandleTable res = make_table(count, current_offset);
				current_offset += count;
				return res;
			}


			template<class ...Args>
			HandleTable create_table(Args... t)
			{
				const UINT count = sizeof...(t);
				std::lock_guard<std::mutex> g(m);

				if (frees[count].size())
				{
					auto&& list = frees[count];
					size_t offset = list.front();
					list.pop_front();
					return make_table(count, offset);
				}

				if (current_offset >= max_count)
					Log::get() << Log::LEVEL_ERROR << "DescriptorHeap limit reached!" << Log::endl;

				HandleTable res = make_table(count, current_offset);
				current_offset += count;
				place(res, 0, t...);
				return res;
			}



			Handle handle(UINT offset)
			{
				Handle res;
				res.offset = offset;
				res.heap = this;
				return res;
			}
			/*
			HandleTable get_table_view(UINT offset, UINT count)
			{
				assert((offset + count) <= max_count && "Not enought handles");
				HandleTable res;
				res.info = std::shared_ptr<HandleTable::helper>(new HandleTable::helper);
				res.info->offset_cpu = offset;
				if (flags == DescriptorHeapFlags::SHADER_VISIBLE)
				res.info->offset_gpu = offset;
				res.info->count = count;
				res.info->heap = this;
				return res;
			}
			*/
			HandleTableLight get_light_table_view(UINT offset, UINT count)
			{
				assert((offset + count) <= max_count && "Not enought handles");
				HandleTableLight res;
				res.offset = offset;
				res.count = count;
				res.heap = this;
				return res;
			}

		};

		class DescriptorPage;


		class DescriptorHeapPaged :public DescriptorHeap
		{
			CommonAllocator allocator;

			UINT offset = 0;
			//using DescriptorHeap::create_table;
		public:
			using ptr = std::shared_ptr<DescriptorHeapPaged>;

			DescriptorHeapPaged(UINT num, DescriptorHeapType type, DescriptorHeapFlags flags = DescriptorHeapFlags::NONE) : DescriptorHeap(num, type, flags), allocator(num / 32)
			{

			}

			~DescriptorHeapPaged()
			{
			}
			std::mutex m;

			DescriptorPage* create_page(UINT count = 1);

			void free_page(DescriptorPage* page);

			size_t used_size()
			{
				return allocator.get_max_usage();
			}

		};
		class DescriptorPage
		{

			//	std::mutex m;
			friend class DescriptorHeapPaged;
			UINT heap_offset;
			UINT count;
			UINT offset;
			DescriptorHeapPaged::ptr heap;

			CommonAllocator::Handle alloc_handle;
		public:
			DescriptorPage(DescriptorHeapPaged::ptr heap, CommonAllocator::Handle alloc_handle, UINT heap_offset, UINT count) :heap(heap), heap_offset(heap_offset), count(count), alloc_handle(alloc_handle)
			{
				offset = 0;
			}

			~DescriptorPage()
			{
			}
			bool has_free_size(UINT need_count = 1)
			{
				return offset + need_count < count;
			}


			Handle place();
			HandleTableLight place(UINT count);
			HandleTable place2(UINT count);
			void free();
		};
		class DescriptorHeapManager : public Singleton<DescriptorHeapManager>
		{
			friend class Singleton<DescriptorHeapManager>;
			DescriptorHeap::ptr heap_rt;
			DescriptorHeap::ptr heap_ds;


			DescriptorHeapManager();

		public:

			enum_array<DescriptorHeapType, DescriptorHeapPaged::ptr> heaps;


			DescriptorHeapPaged::ptr get_cpu_heap(DescriptorHeapType type)
			{
				return heaps[type];
			}

			DescriptorHeapPaged::ptr get_gpu_heap(DescriptorHeapType type)
			{
				return heaps[type];
			}

			DescriptorHeap::ptr& get_rt()
			{
				return heap_rt;
			}

			DescriptorHeapPaged::ptr& get_samplers()
			{
				return heaps[DescriptorHeapType::SAMPLER];
			}
		};

		//template<class LockPolicy = Thread::Free>
		class StaticDescriptors : public Singleton<StaticDescriptors>
		{
			using LockPolicy = Thread::Lockable;
			DescriptorHeapType type;
			DescriptorHeapFlags flags = DescriptorHeapFlags::NONE;

			friend class CommandList;
			friend class GraphicsContext;
			friend class ComputeContext;
			template<class LockPolicy >
			friend class GPUCompiledManager;
			std::list<DescriptorPage*> pages;

			typename LockPolicy::mutex m;
			void create_heap(UINT count)
			{
				if (flags == DescriptorHeapFlags::SHADER_VISIBLE)
				{
					pages.push_back(DescriptorHeapManager::get().get_gpu_heap(type)->create_page(count));
				}
				else
				{
					pages.push_back(DescriptorHeapManager::get().get_cpu_heap(type)->create_page(count));
				}
			}

			HandleTable prepare(UINT count)
			{
				typename LockPolicy::guard g(m);

				if (pages.empty() || !pages.back()->has_free_size(count))
				{
					create_heap(count);
				}

				return pages.back()->place2(count);
			}

		public:

			using ptr = std::shared_ptr<StaticDescriptors>;
			StaticDescriptors()// DescriptorHeapType type, DescriptorHeapFlags flags) :type(type), flags(flags)
			{
				type = DescriptorHeapType::CBV_SRV_UAV;
				flags = DescriptorHeapFlags::SHADER_VISIBLE;
			}

			~StaticDescriptors()
			{
				reset();
			}

			HandleTable place(UINT count)
			{
				return prepare(count);
			}

		};


		template<class LockPolicy = Thread::Free>
		class DynamicDescriptor
		{
			DescriptorHeapType type;
			DescriptorHeapFlags flags = DescriptorHeapFlags::NONE;

			friend class CommandList;
			friend class GraphicsContext;
			friend class ComputeContext;
			template<class LockPolicy >
			friend class GPUCompiledManager;
			std::list<DescriptorPage*> pages;

			typename LockPolicy::mutex m;
			void create_heap(UINT count)
			{
				if (flags == DescriptorHeapFlags::SHADER_VISIBLE)
				{
					pages.push_back(DescriptorHeapManager::get().get_gpu_heap(type)->create_page(count));
				}
				else
				{
					pages.push_back(DescriptorHeapManager::get().get_cpu_heap(type)->create_page(count));
				}
			}



			void reset()
			{
				typename LockPolicy::guard g(m);

				for (auto& p : pages)
					p->free();

				pages.clear();
			}


			HandleTableLight prepare(UINT count)
			{
				typename LockPolicy::guard g(m);

				if (pages.empty() || !pages.back()->has_free_size(count))
				{
					create_heap(count);
				}

				return pages.back()->place(count);
			}

		public:

			using ptr = std::shared_ptr<DynamicDescriptor<LockPolicy>>;
			DynamicDescriptor(DescriptorHeapType type, DescriptorHeapFlags flags) :type(type), flags(flags)
			{

			}

			~DynamicDescriptor()
			{
				reset();
			}

			Handle place(Handle e)
			{
				auto table = place();
				table.place(e);
				return table;
			}

			HandleTableLight place(std::initializer_list<Handle> list)
			{
				auto table = prepare((UINT)list.size());

				int i = 0;
				for (auto& e : list)
					table[i++].place(e);

				return table;
			}


			HandleTableLight place(UINT count)
			{
				return prepare(count);
			}

			template<class T = Handle>
			T place()
			{
				return T(prepare(1)[0]);
			}
		};


		D3D12_CPU_DESCRIPTOR_HANDLE Handle::get_cpu()const
		{
			return heap->native_heap[offset].get_cpu();
		}

		D3D12_GPU_DESCRIPTOR_HANDLE Handle::get_gpu()const
		{
			return heap->native_heap[offset].get_gpu();
		}


		template<HAL::Views::ViewTemplate T>
		void Handle::operator=(const T& v)
		{
			heap->native_heap[offset].place(v);

			if constexpr (std::is_same_v < T, HAL::Views::UnorderedAccess>)
			{
				if (auto buffer = std::get_if<HAL::Views::UnorderedAccess::Buffer>(&v.View))
					*get_resource_info() = ResourceInfo(static_cast<Resource*>(v.Resource), buffer->CounterResource ? static_cast<Resource*>(buffer->CounterResource) : nullptr, v);
				else
					*get_resource_info() = ResourceInfo(static_cast<Resource*>(v.Resource), nullptr, v);
			}
			else
			{
				*get_resource_info() = ResourceInfo(static_cast<Resource*>(v.Resource), v);
			}

		}
	}

}