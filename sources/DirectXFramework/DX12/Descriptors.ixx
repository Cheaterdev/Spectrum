module;

export module Descriptors;


import Allocators;
import Vectors;


import stl.threading;
import stl.core;

import Log;
import Data;
import d3d12_types;
import d3d12;
export
{

	namespace DX12
	{
		//	class CommandList;


		DescriptorHeapType get_heap_type(HandleType type);
		class Resource;



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

		struct Handle
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpu;
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpu;

			ResourceInfo* resource_info = nullptr;

			bool is_valid() const
			{
				return cpu.ptr != 0 || gpu.ptr != 0;
			}

			operator bool()
			{
				return !!resource_info;
			}
			Handle()
			{
				gpu.ptr = 0;
				cpu.ptr = 0;
			}

			void operator=(const std::function<void(Handle&)>& f)
			{
				f(*this);
			}

			UINT offset = UINT_MAX;

			//const Handle& operator=(const Handle& h) = default;

			bool operator!=(const Handle& r)
			{
				if (cpu.ptr != r.cpu.ptr) return true;

				if (gpu.ptr != r.gpu.ptr) return true;

				return false;
			}

			void place(const Handle& r) const;

			//	void clear(CommandList& list, float4 = { 0, 0, 0, 0 }) const;
		};

		namespace HLSL
		{
			struct RaytracingAccelerationStructure : public Handle
			{
				static const HandleType TYPE = HandleType::SRV;

				RaytracingAccelerationStructure() = default;
				RaytracingAccelerationStructure(const Handle& h) : Handle(h)
				{

				}

				void create(Resource* resource);
			};

			template<class T>
			struct StructuredBuffer : public Handle
			{
				static const HandleType TYPE = HandleType::SRV;

				StructuredBuffer() = default;
				explicit  StructuredBuffer(const Handle& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT first_elem = 0, UINT count = 0);
			};

			template<class T>
			struct RWStructuredBuffer : public Handle
			{
				static const HandleType TYPE = HandleType::UAV;

				RWStructuredBuffer() = default;
				explicit  RWStructuredBuffer(const Handle& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT first_elem = 0, UINT count = 0);
			};

			template<class T>
			struct AppendStructuredBuffer : public Handle
			{
				static const HandleType TYPE = HandleType::UAV;

				AppendStructuredBuffer() = default;
				explicit  AppendStructuredBuffer(const Handle& h) : Handle(h)
				{

				}

				void create(Resource* counter_resource, UINT counter_offset, Resource* resource, UINT first_elem = 0, UINT count = 0);
			};

			struct ByteAddressBuffer : public Handle
			{
				static const HandleType TYPE = HandleType::SRV;

				ByteAddressBuffer() = default;
				explicit ByteAddressBuffer(const Handle& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT offset = 0, UINT size = 0);
			};


			struct RWByteAddressBuffer : public Handle
			{
				static const HandleType TYPE = HandleType::UAV;

				RWByteAddressBuffer() = default;
				explicit RWByteAddressBuffer(const Handle& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT offset = 0, UINT size = 0);
			};

			template<class T>
			struct Buffer : public Handle
			{
				static const HandleType TYPE = HandleType::SRV;

				Buffer() = default;
				explicit  Buffer(const Handle& h) : Handle(h)
				{

				}

				void create(Resource* resource, DXGI_FORMAT format, UINT offset = 0, UINT size = 0);
			};

			template<class T>
			struct RWBuffer : public Handle
			{
				static const HandleType TYPE = HandleType::UAV;

				RWBuffer() = default;
				explicit RWBuffer(const Handle& h) : Handle(h)
				{

				}
				void create(Resource* resource, DXGI_FORMAT format, UINT offset = 0, UINT size = 0);
			};

			template<class T = float4>
			struct Texture2D : public Handle
			{
				static const HandleType TYPE = HandleType::SRV;


				Texture2D() = default;
				explicit Texture2D(const Handle& h) : Handle(h)
				{

				}

				template<class T2>
				Texture2D(const Texture2D<T2>& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT first_mip = 1, UINT mip_levels = 0, UINT array_offset = 0);
			};

			template<class T = float4>
			struct Texture2DArray : public Handle
			{
				static const HandleType TYPE = HandleType::SRV;

				Texture2DArray() = default;
				explicit Texture2DArray(const Handle& h) : Handle(h)
				{

				}

				template<class T2>
				Texture2DArray(const Texture2DArray<T2>& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT first_mip = 1, UINT mip_levels = 0, UINT array_offset = 0, UINT array_count = 1);
			};

			template<class T = float4>
			struct Texture3D : public Handle
			{
				static const HandleType TYPE = HandleType::SRV;

				Texture3D() = default;
				explicit Texture3D(const Handle& h) : Handle(h)
				{

				}

				template<class T2>
				Texture3D(const Texture3D<T2>& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT first_mip = 1, UINT mip_levels = 0, UINT array_offset = 0);
			};
			template<class T = float4>
			struct TextureCube : public Handle
			{
				static const HandleType TYPE = HandleType::SRV;

				TextureCube() = default;
				explicit TextureCube(const Handle& h) : Handle(h)
				{

				}

				template<class T2>
				TextureCube(const TextureCube<T2>& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT first_mip = 1, UINT mip_levels = 0, UINT array_offset = 0);
			};

			template<class T = float4>
			struct RWTexture2D : public Handle
			{
				static const HandleType TYPE = HandleType::UAV;
				RWTexture2D() = default;
				explicit RWTexture2D(const Handle& h) : Handle(h)
				{

				}

				template<class T2>
				RWTexture2D(const RWTexture2D<T2>& h) : Handle(h)
				{

				}
				void create(Resource* resource, UINT first_mip = 1, UINT array_offset = 0);
			};

			template<class T = float4>
			struct RWTexture3D : public Handle
			{
				static const HandleType TYPE = HandleType::UAV;

				RWTexture3D() = default;
				explicit RWTexture3D(const Handle& h) : Handle(h)
				{

				}
				template<class T2>
				RWTexture3D(const RWTexture3D<T2>& h) : Handle(h)
				{

				}

				void create(Resource* resource, UINT first_mip = 1, UINT array_offset = 0);

			};

			template<class T = float4>
			struct RenderTarget : public Handle
			{
				static const HandleType TYPE = HandleType::RTV;

				RenderTarget() = default;
				explicit RenderTarget(const Handle& h) : Handle(h)
				{

				}

				template<class T2>
				RenderTarget(const RenderTarget<T2>& h) : Handle(h)
				{

				}

				void createFrom2D(Resource* resource, UINT mip_offset = 0);
				void createFrom2DArray(Resource* resource, UINT mip_offset = 0, UINT array_offset = 0);

			};

			template<class T = float4>
			struct DepthStencil : public Handle
			{
				static const HandleType TYPE = HandleType::DSV;

				DepthStencil() = default;
				explicit DepthStencil(const Handle& h) : Handle(h)
				{

				}

				template<class T2>
				DepthStencil(const RenderTarget<T2>& h) : Handle(h)
				{

				}

				void createFrom2D(Resource* resource, UINT mip_offset = 0);
				void createFrom2DArray(Resource* resource, UINT mip_offset = 0, UINT array_offset = 0);

			};
		}
		class DescriptorHeap;
		//	class CommandList;
		struct HandleTable
		{
			Handle operator[](UINT i) const
			{
				assert(i < info->count);
				Handle res = get_base();
				res.cpu.Offset(i, info->descriptor_size);
				res.gpu.Offset(i, info->descriptor_size);
				res.resource_info += i;
				res.offset += i;
				return res;
			}

			Handle& get_base() const
			{
				return info->base;
			}

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
				Handle base;
				UINT descriptor_size;
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
				res.cpu.Offset(i, descriptor_size);
				res.gpu.Offset(i, descriptor_size);
				res.resource_info = resource_info + i;
				res.offset += i;
				return res;
			}


			inline UINT get_count() const
			{
				return count;
			}

			bool valid() const
			{
				return descriptor_size > 0;
			}


		private:
			friend class DescriptorHeap;

			UINT descriptor_size = 0;
			UINT count = 0;

		};
		class DescriptorHeap : public std::enable_shared_from_this<DescriptorHeap>
		{
			//      std::vector<Handle> handles;
			ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
			std::vector<ResourceInfo> resources;
			DescriptorHeapFlags flags;

			UINT descriptor_size;
			UINT current_offset = 0;
			UINT max_count;
			std::mutex m;
			std::map<UINT, std::deque<UINT>> frees;
			HandleTable make_table(UINT count, UINT offset)
			{
				HandleTable res;
				std::weak_ptr<DescriptorHeap> ptr = shared_from_this();
				res.info = std::shared_ptr<HandleTable::helper>(new HandleTable::helper, [ptr, count, offset](HandleTable::helper* e)
					{
						auto t = ptr.lock();
						if (t)
						{
							std::lock_guard<std::mutex> g(t->m);


							t->frees[count].push_back(offset);
						}

						delete e;
					});
				res.info->base.cpu = get_handle(offset);
				res.info->base.gpu = get_gpu_handle(offset);
				res.info->base.resource_info = resources.data() + offset;
				res.info->descriptor_size = descriptor_size;
				res.info->count = count;
				res.info->base.offset = offset;
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

		public:
			using ptr = std::shared_ptr<DescriptorHeap>;

			ComPtr<ID3D12DescriptorHeap> get_native()
			{
				return m_rtvHeap;
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

				if (frees[count].size())
				{
					auto&& list = frees[count];
					UINT offset = list.front();
					list.pop_front();
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

			CD3DX12_CPU_DESCRIPTOR_HANDLE get_handle(UINT i);
			CD3DX12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(UINT i);


			Handle handle(UINT offset)
			{
				Handle res;
				res.cpu = get_handle(offset);
				res.gpu = get_gpu_handle(offset);
				res.resource_info = resources.data() + offset;
				return res;
			}

			HandleTable get_table_view(UINT offset, UINT count)
			{
				assert((offset + count) <= max_count && "Not enought handles");
				HandleTable res;
				res.info = std::shared_ptr<HandleTable::helper>(new HandleTable::helper);

				res.info->base.cpu = get_handle(offset);
				res.info->base.gpu = get_gpu_handle(offset);
				res.info->base.resource_info = resources.data() + offset;
				res.info->descriptor_size = descriptor_size;
				res.info->count = count;
				res.info->base.offset = offset;


				return res;
			}

			HandleTableLight get_light_table_view(UINT offset, UINT count)
			{
				assert((offset + count) <= max_count && "Not enought handles");
				HandleTableLight res;

				res.cpu = get_handle(offset);
				res.gpu = get_gpu_handle(offset);
				res.descriptor_size = descriptor_size;
				res.resource_info = resources.data() + offset;
				res.count = count;
				res.offset = offset;

				return res;
			}

		};

		class DescriptorPage;


		class DescriptorHeapPaged :public DescriptorHeap
		{
			CommonAllocator allocator;

			UINT offset = 0;

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
			void free();
		};
		class DescriptorHeapManager : public Singleton<DescriptorHeapManager>
		{
			friend class Singleton<DescriptorHeapManager>;
			DescriptorHeap::ptr heap_cb_sr_ua_static;
			DescriptorHeap::ptr heap_rt;
			DescriptorHeap::ptr heap_ds;


			DescriptorHeapManager();

		public:

			enum_array<DescriptorHeapType, DescriptorHeapPaged::ptr> cpu_heaps;
			enum_array<DescriptorHeapType, DescriptorHeapPaged::ptr> gpu_heaps;


			DescriptorHeapPaged::ptr get_cpu_heap(DescriptorHeapType type)
			{
				return cpu_heaps[type];
			}

			DescriptorHeapPaged::ptr get_gpu_heap(DescriptorHeapType type)
			{
				return gpu_heaps[type];
			}

			DescriptorHeap::ptr& get_csu_static()
			{
				return heap_cb_sr_ua_static;
			}
			DescriptorHeap::ptr& get_rt()
			{
				return heap_rt;
			}

			DescriptorHeapPaged::ptr& get_samplers()
			{
				return cpu_heaps[DescriptorHeapType::SAMPLER];
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