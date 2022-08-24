module;
#include "helper.h"
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
import HAL.Types;
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

			HandleType type;
			union
			{
				D3D12_DEPTH_STENCIL_VIEW_DESC dsv;

				struct
				{
					Resource* counter;
					D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
				} uav;

				D3D12_SHADER_RESOURCE_VIEW_DESC srv;
				D3D12_RENDER_TARGET_VIEW_DESC rtv;
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;

			};

			ResourceInfo() = default;

			ResourceInfo(Resource* resource_ptr, D3D12_SHADER_RESOURCE_VIEW_DESC srv) :resource_ptr(resource_ptr)
			{
				type = HandleType::SRV;
				this->srv = srv;
			}

			ResourceInfo(Resource* resource_ptr, D3D12_DEPTH_STENCIL_VIEW_DESC dsv) :resource_ptr(resource_ptr)
			{
				type = HandleType::DSV;
				this->dsv = dsv;
			}

			ResourceInfo(Resource* resource_ptr, Resource* counter_ptr, D3D12_UNORDERED_ACCESS_VIEW_DESC uav) :resource_ptr(resource_ptr)
			{
				type = HandleType::UAV;
				this->uav.desc = uav;
				this->uav.counter = counter_ptr;
			}

			ResourceInfo(Resource* resource_ptr, D3D12_RENDER_TARGET_VIEW_DESC rtv) :resource_ptr(resource_ptr)
			{
				type = HandleType::RTV;
				this->rtv = rtv;
			}

			ResourceInfo(Resource* resource_ptr, D3D12_CONSTANT_BUFFER_VIEW_DESC cbv) :resource_ptr(resource_ptr)
			{
				type = HandleType::CBV;
				this->cbv = cbv;
			}

			void for_each_subres(std::function<void(Resource*, UINT)> f) const;
		};

		//class DescriptorHeap;
		struct Handle
		{
			DescriptorHeap* heap = nullptr;

			ResourceInfo* get_resource_info() const;

			CD3DX12_CPU_DESCRIPTOR_HANDLE get_cpu_read() const; 
			CD3DX12_CPU_DESCRIPTOR_HANDLE get_cpu() const;
			CD3DX12_GPU_DESCRIPTOR_HANDLE get_gpu() const;

			bool is_valid() const
			{
				return offset_gpu != UINT_MAX || offset_cpu != UINT_MAX;
			}

			operator bool() const;
			void operator=(const std::function<void(Handle&)>& f)
			{
				f(*this);
			}

			UINT offset_gpu = UINT_MAX;
			UINT offset_cpu = UINT_MAX;
	
			bool operator!=(const Handle& r)
			{
				if (offset_gpu != r.offset_gpu) return true;

				if (offset_cpu != r.offset_cpu) return true;

				return false;
			}

			void place(const Handle& r) const;

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
				if(info->offset_cpu!=UINT_MAX) res.offset_cpu = info->offset_cpu + i;
				if (info->offset_gpu != UINT_MAX)res.offset_gpu = info->offset_gpu + i;
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
				DescriptorHeap *heap;
				UINT offset_cpu;
				UINT offset_gpu;
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
				if (offset_cpu != UINT_MAX)  res.offset_cpu += i;
				if (offset_gpu != UINT_MAX)  res.offset_gpu += i;
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
			//      std::vector<Handle> handles;
			ComPtr<ID3D12DescriptorHeap> m_MainHeap;
			ComPtr<ID3D12DescriptorHeap> m_CPUHeap; 
			std::vector<ResourceInfo> resources;
			DescriptorHeapFlags flags;

			UINT descriptor_size;
			UINT current_offset = 0;
			UINT max_count;
			std::mutex m;
			std::map<UINT, std::deque<UINT>> frees;
			UINT used_size = 0;
		public:
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
				res.info->offset_cpu = offset;
				res.info->offset_gpu = offset;
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

			CD3DX12_CPU_DESCRIPTOR_HANDLE get_handle(UINT i);
			CD3DX12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(UINT i);
	CD3DX12_CPU_DESCRIPTOR_HANDLE get_cpu_read(UINT i);
		
			friend struct Handle;

			ResourceInfo* get_resource_info(const Handle* h)
			{
				return &resources[h->offset_cpu];
			}
		public:
			using ptr = std::shared_ptr<DescriptorHeap>;

			ComPtr<ID3D12DescriptorHeap> get_native()
			{
				return m_MainHeap;
			}

			ComPtr<ID3D12DescriptorHeap> get_cpu_native()
			{
				return m_CPUHeap;
			}

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
				res.offset_cpu = offset;
				if (flags == DescriptorHeapFlags::SHADER_VISIBLE)
				res.offset_gpu = offset;
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

				res.offset_cpu = offset;
				if (flags == DescriptorHeapFlags::SHADER_VISIBLE)
					res.offset_gpu = offset;
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
		class StaticDescriptors: public Singleton<StaticDescriptors>
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

	}

}