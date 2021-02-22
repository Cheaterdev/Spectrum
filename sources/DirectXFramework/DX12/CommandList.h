#pragma once

#include "CommandListCompiler.h"
enum class Layouts;
namespace DX12
{

	class PipelineState;
	template<class T> class  PipelineStateTyped;
	class ComputePipelineState;
	class PipelineStateBase;

	class IndexBufferBase;
	class UploadBuffer;
	class CPUBuffer;

	class GPUBuffer;

	template<class T> class StructuredBuffer;
	//  using BufferBase = GPUBuffer;
	class QueryHeap;

	class Texture;
	class CommandList;

#define DEFAULT_ALIGN 256

	
	class GraphicsContext;
	class ComputeContext;
	class CopyContext;
	class FrameResourceManager;

	struct UploadInfo
	{
		std::shared_ptr<Resource> resource;
		UINT64 offset;
		UINT64 size;
		D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address();
		ResourceAddress get_resource_address();
		Handle create_cbv(CommandList& list);

		std::byte* get_cpu_data() const;
	};
	
	template<class LockPolicy>
	class Uploader
	{
		ResourceHeapAllocator<LockPolicy> allocator;
		friend class BufferCache;
		std::list<ResourceHandle> handles;
	//	std::vector<std::shared_ptr<UploadBuffer>> upload_resources;
//		UINT64 resource_offset;
		UINT heap_size = 0x200000;
	protected:
		//void reset();

		template<class T>
		size_t size_of(std::span<T>& elem)
		{
			return sizeof(T) * elem.size();
		}
		template<class T>
		size_t size_of(std::vector<T>& elem)
		{
			return sizeof(T) * elem.size();
		}

		template<class T>
		size_t size_of(my_unique_vector<T>& elem)
		{
			return sizeof(T) * elem.size();
		}

		template<class T>
		size_t size_of(T& elem)
		{
			return sizeof(T);
		}

		typename LockPolicy::mutex m;

	public:
		

	//	UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN);


		void reset()
		{
			typename LockPolicy::guard g(m);

			for (auto& h : handles)
				h.Free();
			handles.clear();


		}


		UploadInfo place_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN)
		{
			const auto AlignedSize = static_cast<UINT>(Math::roundUp(uploadBufferSize, alignment));


			auto handle = allocator.alloc(AlignedSize, alignment, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS, HeapType::UPLOAD);

			//	auto handle = BufferCache::get().get_upload(AlignedSize, alignment);
			typename LockPolicy::guard g(m);

			handles.emplace_back(handle);
			UploadInfo info;
			info.resource = handle.get_heap()->cpu_buffer;
			info.offset = handle.get_offset();
			info.size = uploadBufferSize;
			return info;
		}


		
		template<class ...Args>
		UploadInfo place_raw(Args... args)
		{
			size_t size = (0 + ... + size_of(args));



			auto info = place_data(size);


			//	memcpy(info.resource->get_data() + info.offset, data, size);
			size_t start = 0;
			{
				(write(info, start, std::forward<Args>(args)), ...);
			}
			return info;
		}

		void write(UploadInfo& info, size_t offset, void* data, size_t size)
		{
			memcpy(info.resource->buffer_data + info.offset + offset, data, size);
		}



	
		
		//void write(UploadInfo& info, size_t offset, void* data, size_t size);

		template<class T>
		void write(UploadInfo& info, const std::span<T>& arg)
		{
			write(info,0, (void*)arg.data(), arg.size() * sizeof(T));
		}
		template<class T>
		void write(UploadInfo& info, const std::vector<T>& arg)
		{
			write(info,0, (void*)arg.data(), arg.size() * sizeof(T));
		}

		template<class T>
		void write(UploadInfo& info, const my_unique_vector<T>& arg)
		{
			write(info,0, (void*)arg.data(), arg.size() * sizeof(T));
		}

		template<class T>
		void write(UploadInfo& info, size_t& offset, const std::vector<T>& arg)
		{
			write(info, offset, (void*)arg.data(), arg.size() * sizeof(T));
			offset += arg.size() * sizeof(T);
		}


		template<class T>
		void write(UploadInfo& info, size_t& offset, const my_unique_vector<T>& arg)
		{
			write(info, offset, (void*)arg.data(), arg.size() * sizeof(T));
			offset += arg.size() * sizeof(T);

		}
		template<class T>
		void write(UploadInfo& info, size_t& offset, const T& arg)
		{
			write(info, offset, (void*)&arg, sizeof(T));
			offset += sizeof(T);

			//Log::get() << "write " <<  Log::endl;
		}
	};


	class Readbacker
	{
		ResourceHeapAllocator<Thread::Free> allocator;
		friend class BufferCache;
		std::vector<ResourceHandle> handles;
	protected:
		void reset();
	public:

		struct ReadBackInfo
		{
			std::shared_ptr<Resource> resource;
			UINT64 offset;
			UINT64 size;

			std::byte* get_cpu_data() const;
		};

		ReadBackInfo read_data(UINT64 uploadBufferSize, unsigned int alignment = DEFAULT_ALIGN);

	};


	template<class LockPolicy = Thread::Free>
	class GPUCompiledManager : public Uploader<LockPolicy>
	{
		enum_array<DescriptorHeapType, typename DynamicDescriptor<LockPolicy>::ptr> cpu_heaps;
		enum_array<DescriptorHeapType, typename DynamicDescriptor<LockPolicy>::ptr> gpu_heaps;

	public:

		DynamicDescriptor<LockPolicy>& get_cpu_heap(DescriptorHeapType type)
		{
			assert(cpu_heaps[type]);
			return *cpu_heaps[type];
		}

		DynamicDescriptor<LockPolicy>& get_gpu_heap(DescriptorHeapType type)
		{
			assert(cpu_heaps[type]);
			return *gpu_heaps[type];
		}

		GPUCompiledManager()
		{
			gpu_heaps[DescriptorHeapType::CBV_SRV_UAV] = std::make_shared<DynamicDescriptor<LockPolicy>>(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::SHADER_VISIBLE);
			gpu_heaps[DescriptorHeapType::SAMPLER] = std::make_shared<DynamicDescriptor<LockPolicy>>(DescriptorHeapType::SAMPLER, DescriptorHeapFlags::SHADER_VISIBLE);

			cpu_heaps[DescriptorHeapType::CBV_SRV_UAV] = std::make_shared<DynamicDescriptor<LockPolicy>>(DescriptorHeapType::CBV_SRV_UAV, DescriptorHeapFlags::NONE);
			cpu_heaps[DescriptorHeapType::RTV] = std::make_shared<DynamicDescriptor<LockPolicy>>(DescriptorHeapType::RTV, DescriptorHeapFlags::NONE);
			cpu_heaps[DescriptorHeapType::DSV] = std::make_shared<DynamicDescriptor<LockPolicy>>(DescriptorHeapType::DSV, DescriptorHeapFlags::NONE);
			cpu_heaps[DescriptorHeapType::SAMPLER] = std::make_shared<DynamicDescriptor<LockPolicy>>(DescriptorHeapType::SAMPLER, DescriptorHeapFlags::NONE);
		}

		void reset()
		{

			for (auto& h : gpu_heaps)
				if (h)
					h->reset();

			for (auto& h : cpu_heaps)
				if (h)
					h->reset();

			Uploader<LockPolicy>::reset();
		}

	};



	class StaticCompiledGPUData :public Singleton<StaticCompiledGPUData>, public GPUCompiledManager<Thread::Lockable>
	{
	public:
		using Uploader<Thread::Lockable>::place_raw;
	};
	class FrameResources :public std::enable_shared_from_this<FrameResources>, public GPUCompiledManager<Thread::Lockable>
	{

		friend class FrameResourceManager;

		std::uint64_t frame_number = 0;
		std::mutex m;



	public:




		~FrameResources()
		{
			reset();
		}
		using ptr = std::shared_ptr<FrameResources>;
		std::uint64_t get_frame()
		{
			return frame_number;
		}

		std::shared_ptr<CommandList> start_list(std::string name = "", CommandListType type = CommandListType::DIRECT);

	};

	class FrameResourceManager :public Singleton<FrameResourceManager>
	{
		std::atomic_size_t frame_number = 0;

	public:
		FrameResources::ptr begin_frame();
	};

	class CommandListBase
	{
	protected:
		int id = -1;
		std::uint64_t global_id;
		CommandListType type;

		LEAK_TEST(CommandListBase)

		std::vector<std::function<void()>> on_execute_funcs;

		std::list<FenceWaiter> waits;
		
		CommandListCompilerDelayed compiler;


		CommandListCompilerDelayed* get_native_list()
		{
			return &compiler;
		}
	public:
	
		virtual ~CommandListBase() = default;

		CommandListType get_type()
		{
			return type;
		}
	};

	class TransitionCommandList;

	class Transitions : public virtual CommandListBase
	{
		std::vector<D3D12_RESOURCE_BARRIER> transitions;
		//unsigned int transition_count = 0;

		std::list<Resource*> used_resources;
		std::list<TrackedResource::ptr> tracked_resources;
	
		std::shared_ptr<TransitionCommandList> transition_list;

		
	protected:
		void reset();
		std::list<ComPtr<ID3D12PipelineState>> tracked_psos;
		
	public:
		void free_resources();
		std::list<ComPtr<ID3D12Heap>> tracked_heaps;
		void flush_transitions();

		void transition(const Resource* resource, ResourceState state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		void transition(const Resource::ptr& resource, ResourceState state, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
		void use_resource(const Resource* resource);
	public:
		void prepare_transitions(Transitions* to, bool all);

		void transition_uav(Resource* resource);
		void transition(Resource* from, Resource* to);
		std::shared_ptr<TransitionCommandList> fix_pretransitions();

		void transition_rtv(const ResourceInfo* info)
		{
			assert(info->type == HandleType::RTV);

			auto& desc = info->resource_ptr->get_desc();
			if (info->rtv.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2D)
			{

				if (desc.MipLevels == 1 && desc.Depth() == 1)
				{
					transition(info->resource_ptr, ResourceState::RENDER_TARGET, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					auto& rtv = info->rtv.Texture2D;
					UINT res = desc.CalcSubresource(rtv.MipSlice, 0, rtv.PlaneSlice);
					transition(info->resource_ptr, ResourceState::RENDER_TARGET, res);
				}
			}
			else 	if (info->rtv.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2DARRAY)
			{

				auto& rtv = info->rtv.Texture2DArray;

				for (UINT array = rtv.FirstArraySlice; array < rtv.FirstArraySlice + rtv.ArraySize; array++)
				{
					UINT res = desc.CalcSubresource(rtv.MipSlice, array, rtv.PlaneSlice);
					transition(info->resource_ptr, ResourceState::RENDER_TARGET, res);
				}
			}

		}

		void transition_uav(const ResourceInfo* info)
		{
			assert(info->type == HandleType::UAV);


			auto& desc = info->resource_ptr->get_desc();

			if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_BUFFER)
			{
				transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				if (info->uav.counter) transition(info->uav.counter, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			}
			else if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE2D)
			{
				auto& uav = info->uav.desc.Texture2D;

				if (desc.MipLevels == 1 && desc.Depth() == 1)
				{
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					UINT res = desc.CalcSubresource(uav.MipSlice, 0, uav.PlaneSlice);
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, res);
				}
			}
			else if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE3D)
			{
				auto& uav = info->uav.desc.Texture3D;
				if (uav.FirstWSlice == 0 && uav.WSize == desc.Depth() && desc.MipLevels == 1)
				{
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					UINT res = desc.CalcSubresource(uav.MipSlice, 0, 0);
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, res);
				}
			}

			else if (info->uav.desc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE2DARRAY)
			{
				auto& uav = info->uav.desc.Texture2DArray;

				if (desc.MipLevels == 1 && desc.Depth() == 1 && desc.ArraySize() == 1)
				{
					transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{

					for (UINT array = uav.FirstArraySlice; array < uav.FirstArraySlice + uav.ArraySize; array++)
					{
						UINT res = desc.CalcSubresource(uav.MipSlice, array, uav.PlaneSlice);
						transition(info->resource_ptr, ResourceState::UNORDERED_ACCESS, res);
					}

				}
			}
			else assert(false);

		}


		void transition_dsv(const ResourceInfo* info)
		{
			assert(info->type == HandleType::DSV);

			auto& desc = info->resource_ptr->get_desc();

			if (info->dsv.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D)
			{
				auto& dsv = info->dsv.Texture2D;

				UINT res = desc.CalcSubresource(dsv.MipSlice, 0, 0);
				transition(info->resource_ptr, ResourceState::DEPTH_WRITE, res);

			}
			else if (info->dsv.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2DARRAY)
			{
				auto& dsv = info->dsv.Texture2DArray;

				for (UINT array = dsv.FirstArraySlice; array < dsv.FirstArraySlice + dsv.ArraySize; array++)
				{
					UINT res = desc.CalcSubresource(dsv.MipSlice, array, 0);
					transition(info->resource_ptr, ResourceState::DEPTH_WRITE, res);
				}
			}
			else
				assert(0);
		}


		void transition_srv(const ResourceInfo* info)
		{
			assert(info->type == HandleType::SRV);

			auto& desc = info->resource_ptr->get_desc();
			UINT total = desc.CalcSubresource(desc.MipLevels - 1, desc.ArraySize() - 1, desc.Depth() - 1);

			
			ResourceState target_state = ResourceState::NON_PIXEL_SHADER_RESOURCE;

			if (type == CommandListType::DIRECT)
			{
				target_state = ResourceState::PIXEL_SHADER_RESOURCE | ResourceState::NON_PIXEL_SHADER_RESOURCE;
			}

			if (type == CommandListType::COMPUTE)
			{
				target_state = ResourceState::NON_PIXEL_SHADER_RESOURCE;
			}

			if (type == CommandListType::COPY)
			{
				assert(false);
			}

			if (info->srv.ViewDimension == D3D12_UAV_DIMENSION_BUFFER)
			{
				transition(info->resource_ptr, target_state, 0);// D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			}
			else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2D)
			{
				auto& srv = info->srv.Texture2D;

				if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0 && desc.Depth() == 1)
				{
					transition(info->resource_ptr, target_state, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
					{
						UINT res = desc.CalcSubresource(mip, 0, srv.PlaneSlice);
						transition(info->resource_ptr, target_state, res);
					}
				}

			}
			else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
			{
				auto& srv = info->srv.Texture2DArray;

				if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0 && srv.FirstArraySlice == 0 && srv.ArraySize == desc.ArraySize() && desc.Depth() == 1)
				{
					transition(info->resource_ptr, target_state, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
						for (auto array = srv.FirstArraySlice; array < srv.FirstArraySlice + srv.ArraySize; array++)
						{
							UINT res = desc.CalcSubresource(mip, array, srv.PlaneSlice);
							transition(info->resource_ptr, target_state, res);
						}
				}
			}
			else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURE3D)
			{
				auto& srv = info->srv.Texture3D;

				if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0)
				{
					transition(info->resource_ptr, target_state, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
					{
						UINT res = desc.CalcSubresource(mip, 0, 0);
						transition(info->resource_ptr, target_state, res);
					}
				}
			}
			else if (info->srv.ViewDimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
			{
				auto& srv = info->srv.TextureCube;

				if (srv.MipLevels == desc.MipLevels && srv.MostDetailedMip == 0)
				{
					transition(info->resource_ptr, target_state, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				}
				else
				{
					for (auto mip = srv.MostDetailedMip; mip < srv.MostDetailedMip + srv.MipLevels; mip++)
						for (auto array = 0; array < 6; array++)
						{
							UINT res = desc.CalcSubresource(mip, array, 0);
							transition(info->resource_ptr, target_state, res);
						}
				}
			}
			else
				assert(false);




		}

	};

	class GPUTimeManager;
	class Eventer;

	class GPUTimer
	{
		friend class GPUTimeManager;

		int id;

		
	public:
		CommandListType queue_type;
		GPUTimer();

		virtual ~GPUTimer();

		void start(Eventer* list);

		void end(Eventer* list);

		float get_time();


		double get_start();

		double get_end();
	};


	class GPUCounter
	{

	public:
		GPUTimer timer;
		bool enabled = true;

		std::chrono::time_point<std::chrono::high_resolution_clock>  start_time;
		std::chrono::time_point<std::chrono::high_resolution_clock>  end_time;

	};

	class GPUBlock :public TimedBlock
	{
	public:
		using TimedBlock::TimedBlock;

		GPUCounter gpu_counter;
	};


	class Eventer : public virtual CommandListBase, public TimedRoot
	{
	
 		std::list<std::wstring> names;
		TimedBlock* current;

		virtual  void on_start(Timer* timer) override;
		virtual  void on_end(Timer* timer)override;
	protected:
#ifdef DEV
		Exceptions::stack_trace begin_stack;
#endif
		std::string name;
		void reset();
		void begin(std::string name, Timer* t = nullptr);
	public:
		static thread_local Eventer* thread_current;

		virtual Timer start(std::wstring_view name)override;

		std::shared_ptr<Timer> timer;
		// events
		void start_event(std::wstring str);
		void end_event();

		void set_marker(const wchar_t* label);


		// timers
		void insert_time(QueryHeap& pQueryHeap, uint32_t QueryIdx);
		void resolve_times(QueryHeap& pQueryHeap, uint32_t NumQueries, std::function<void(std::span<UINT64>)>);

	};



	class Sendable : public virtual CommandListBase
	{

		friend class Queue;
		std::vector<std::function<void(FenceWaiter)>> on_fence;


		std::promise<FenceWaiter> execute_fence;
		std::shared_future<FenceWaiter> execute_fence_result;


		std::list<FenceWaiter> waits;

	
		void on_send(FenceWaiter fence)
		{
		
		
			
			execute_fence.set_value(fence);
			
			for (auto&& e : on_fence)
				e(fence);
			
			on_fence.clear();
		}
	protected:
		CommandListCompiled compiled;
	


		
	public:
		void compile();
		void when_send(std::function<void(FenceWaiter)> e)
		{
			on_fence.emplace_back(e);
		}

		std::shared_future<FenceWaiter> get_execution_fence()
		{
			return execute_fence_result;
		}

		std::shared_future<FenceWaiter> execute(std::function<void()> f = nullptr);
		void execute_and_wait(std::function<void()> f = nullptr);

		void on_done(std::function<void()> f);

	};

	class SignatureDataSetter;


	class CommandList : public std::enable_shared_from_this<CommandList>, public Readbacker, public Transitions, public Eventer, public Sendable, public GPUCompiledManager<Thread::Free>
	{


	public:
		using ptr = shared_ptr<CommandList>;
	protected:
		CommandList() = default;
		friend class Queue;
		friend class FrameResourceManager;
		friend class CopyContext;
		friend class GraphicsContext;
		friend class ComputeContext;

		
		// TODO: make references?

		virtual void on_execute();

		bool heaps_changed = false;

		std::array<DescriptorHeap::ptr, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> heaps;

		std::unique_ptr<GraphicsContext> graphics;
		std::unique_ptr<ComputeContext> compute;
		std::unique_ptr<CopyContext> copy;
		PipelineStateBase* current_pipeline;

		std::list<update_tiling_info> tile_updates;


	
		void set_pipeline_internal(PipelineStateBase* pipeline);

		void set_heap(DescriptorHeapType type, DescriptorHeap::ptr heap)
		{
			assert(this->type != CommandListType::COPY);
			auto i_type = static_cast<int>(type);
			if (heaps[i_type] == heap) return;

			heaps_changed = true;
			heaps[i_type] = heap;
			flush_heaps();

		}

	public:	void update_tilings(update_tiling_info&& info)
	{
		tile_updates.emplace_back(std::move(info));

		use_resource(info.resource);
	}

		FrameResources::ptr frame_resources;
		void setup_debug(SignatureDataSetter*);

		void print_debug();

		bool first_debug_log = true;
		std::shared_ptr<GPUBuffer> debug_buffer;

		//	DynamicDescriptor<DescriptorHeapType::CBV_SRV_UAV, Free, DescriptorHeapFlags::SHADER_VISIBLE> srv_descriptors;
		//	DynamicDescriptor<DescriptorHeapType::SAMPLER, Free, DescriptorHeapFlags::SHADER_VISIBLE> smp_descriptors;

		GraphicsContext& get_graphics();
		ComputeContext& get_compute();
		CopyContext& get_copy();



		void flush_heaps(bool force = false)
		{
			assert(type != CommandListType::COPY);
			if (!heaps_changed && !force) return;
			heaps_changed = false;
			std::array<ID3D12DescriptorHeap*, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES - 1> native_heaps;

			int count = 0;
			for (auto& e : heaps)
			{
				if (!e) continue;
				native_heaps[count] = e->get_native().Get();
				count++;
			}
			if (count)
			{
				if (count == 2 && native_heaps[1] == native_heaps[0])
					return;
				compiler.SetDescriptorHeaps(count, native_heaps.data());
			}

		}
		FrameResources* get_manager()
		{
			return frame_resources.get();
		}

		ptr get_ptr()
		{
			return std::static_pointer_cast<CommandList>(shared_from_this());
		}
		CommandList(CommandListType);

		void begin(std::string name = "", Timer* t = nullptr);
		void end();

		int get_id()
		{
			return id;
		}

		UINT64 get_global_id()
		{
			return global_id;
		}

		void clear_uav(const Handle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
		{
			transition_uav(h.resource_info);

			flush_transitions();
			auto handle = get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place(h);
			get_native_list()->ClearUnorderedAccessViewFloat(handle.gpu, h.cpu, h.resource_info->resource_ptr->get_native().Get(), reinterpret_cast<FLOAT*>(ClearColor.data()), 0, nullptr);
		}


		void clear_rtv(const Handle& h, vec4 ClearColor = vec4(0, 0, 0, 0))
		{
			transition_rtv(h.resource_info);
			flush_transitions();
			get_native_list()->ClearRenderTargetView(h.cpu, ClearColor.data(), 0, nullptr);
		}

		template<class T>
		void clear_counter(std::shared_ptr<StructuredBuffer<T>>& buffer)
		{
			clear_uav(buffer->counted_uav[0]);
		}


		void clear_stencil(Handle dsv, UINT8 stencil = 0)
		{
			transition_dsv(dsv.resource_info);
			flush_transitions();
			get_native_list()->ClearDepthStencilView(dsv.cpu, D3D12_CLEAR_FLAG_STENCIL, 0, stencil, 0, nullptr);
		}

		void clear_depth(Handle dsv, float depth = 0)
		{
			transition_dsv(dsv.resource_info);
			flush_transitions();
			get_native_list()->ClearDepthStencilView(dsv.cpu, D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
		}


	};

	class CopyContext
	{
		friend class CommandList;

		CommandList& base;
		CommandListCompilerDelayed* list;

		CopyContext(CommandList& base) :base(base), list(base.get_native_list()) {}
		CopyContext(const CopyContext&) = delete;
		CopyContext(CopyContext&&) = delete;
	public:
		void copy_resource(Resource* dest, Resource* source);
		void copy_resource(const Resource::ptr& dest, const Resource::ptr& source);
		void copy_texture(const Resource::ptr& dest, int, const Resource::ptr& source, int);
		void copy_texture(const Resource::ptr& dest, ivec3, const Resource::ptr& source, ivec3, ivec3);
		void copy_buffer(Resource* dest, int s_dest, Resource* source, int s_source, int size);


		//TODO: remove
		void update_resource(Resource::ptr resource, UINT first_subresource, UINT sub_count, D3D12_SUBRESOURCE_DATA* data);
		void update_buffer(Resource::ptr resource, UINT offset, const char* data, UINT size);
		void update_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);
		void update_buffer(Resource* resource, UINT offset, const char* data, UINT size);
		void update_texture(Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, const char* data, UINT row_stride, UINT slice_stride = 0);



		std::future<bool> read_texture(Resource::ptr resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>);
		std::future<bool> read_texture(const Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, std::function<void(const char*, UINT64, UINT64, UINT64)>);
		std::future<bool> read_buffer(Resource* resource, unsigned int offset, UINT64 size, std::function<void(const char*, UINT64)>);
		std::future<bool> read_query(std::shared_ptr<QueryHeap>&, unsigned int offset, unsigned int count, std::function<void(const char*, UINT64)>);
	};



	class SignatureDataSetter
	{
	protected:
		CommandList& base;
		SignatureDataSetter(CommandList& base) :base(base) {	}
	public:

		CommandList& get_base() {
			return base;
		}
		virtual void set_signature(const RootSignature::ptr&) = 0;
		virtual void set(UINT, const HandleTableLight&) = 0;
		virtual void set_const_buffer(UINT i, const D3D12_GPU_VIRTUAL_ADDRESS&) = 0;

		template<class T>
		std::unique_ptr<T> wrap()
		{
			auto res = std::make_unique<T>();
			res->begin(this);
			return res;
		}
	};

	class GraphicsContext : public SignatureDataSetter
	{
		friend class CommandList;
		CommandListCompilerDelayed* list;

		GraphicsContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {
		}
		GraphicsContext(const GraphicsContext&) = delete;
		GraphicsContext(GraphicsContext&&) = delete;

		template<class T, class W>
		void set(T* _where, W what)
		{
			*_where = what->get_vertex_buffer_view();
			//    what->update(this);
		}

		template<class T, class W, class ...Args>
		void set(T* _where, W what, Args...args)
		{
			*_where = what->get_vertex_buffer_view();
			//what->update(this);
			set(_where++, args...);
		}


		bool valid_scissor = false;
		std::vector<Viewport> viewports;
		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

		//	std::shared_ptr<PipelineState> current_pipeline;
		std::shared_ptr<RootSignature> current_root_signature;


		void begin();
		void end();
		void on_execute();
	public:

		void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&)override;
		
		void set(UINT, const HandleTableLight&)override;


	public:

		CommandList& get_base()
		{
			return base;
		}
		/*
		CommandListCompiler* get_native_list()
		{
			return &list;
		}*/
		void flush_binds(bool force = false);
		void set_topology(D3D_PRIMITIVE_TOPOLOGY topology)
		{
			if (this->topology != topology)
				list->IASetPrimitiveTopology(topology);

			this->topology = topology;
		}


		void set_signature(const RootSignature::ptr&) override;
		void set_pipeline(std::shared_ptr<PipelineState>);




		void set_layout(Layouts layout);

		void set_heaps(DescriptorHeap::ptr& a, DescriptorHeap::ptr& b);

		void set_scissor(sizer_long rect);
		void set_viewport(Viewport viewport);

		void set_viewport(vec4 viewport);

		void set_scissors(sizer_long rect);
		void set_viewports(std::vector<Viewport> viewports);


		void set_rtv(std::initializer_list<Handle> rt, Handle h);

		void set_rtv(const HandleTable&, Handle);
		void set_rtv(int c, Handle rt, Handle h);
	
		void set_rtv(const HandleTableLight&, Handle);

		void draw(D3D12_DRAW_INDEXED_ARGUMENTS args)
		{
			draw_indexed(args.IndexCountPerInstance, args.StartIndexLocation, args.BaseVertexLocation, args.InstanceCount, args.StartInstanceLocation);
		}
		/*
				HandleTable place_uav(UINT count)
				{
					return uav_descriptors.place(count);
				}
				*/

		HandleTableLight place_rtv(UINT count)
		{
			return get_base().get_cpu_heap(DescriptorHeapType::RTV).place(count);
		}

		HandleTableLight place_dsv(UINT count)
		{
			return get_base().get_cpu_heap(DescriptorHeapType::DSV).place(count);
		}

		HandleTableLight place_rtv(std::initializer_list<Handle> list)
		{
			return get_base().get_cpu_heap(DescriptorHeapType::RTV).place(list);
		}

		HandleTableLight place_dsv(std::initializer_list<Handle> list)
		{
			return get_base().get_cpu_heap(DescriptorHeapType::DSV).place(list);
		}


		void set_stencil_ref(UINT ref)
		{
			list->OMSetStencilRef(ref);
		}
		template<class T, size_t N>
		constexpr size_t size(T(&)[N]) { return N; }
		
		template<class ...Handles>
		void set_rtvs(const Handle& h, Handles... rtvlist)
		{

			auto f = [&](Handle h) {
				get_base().transition_rtv(h.resource_info);
			};
		
			(f(std::forward<Handles>(rtvlist)), ...);

			
			if (h.is_valid())
				get_base().transition_dsv(h.resource_info);

			get_base().flush_transitions();

			
			
			CD3DX12_CPU_DESCRIPTOR_HANDLE ar[] = { (rtvlist.cpu)... };
			list->OMSetRenderTargets(size(ar), ar, false, h.is_valid() ? &h.cpu : nullptr);
		}


		std::vector<Viewport> get_viewports()
		{
			return viewports;
		}

		void set_index_buffer(IndexBufferView view)
		{
			get_base().transition(view.resource, ResourceState::INDEX_BUFFER);
			list->IASetIndexBuffer(&view.view);
		}

		void draw(UINT vertex_count, UINT vertex_offset = 0, UINT instance_count = 1, UINT instance_offset = 0);
		void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count = 1, UINT instance_offset = 0);



		void execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset = 0, Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);

	};



	class ComputeContext : public SignatureDataSetter
	{
		friend class CommandList;


		CommandListCompilerDelayed* list;

		ComputeContext(CommandList& base) :SignatureDataSetter(base), list(base.get_native_list()) {}
		ComputeContext(const ComputeContext&) = delete;
		ComputeContext(ComputeContext&&) = delete;


		//	std::shared_ptr<ComputePipelineState> current_compute_pipeline;
		std::shared_ptr<RootSignature> current_compute_root_signature;

		void begin();
		void end();
		void on_execute();




		void set(UINT, const HandleTableLight&)override;




	
		
		virtual void set_const_buffer(UINT, const D3D12_GPU_VIRTUAL_ADDRESS&) override;


	public:

		CommandList& get_base()
		{
			return base;
		}

		void flush_binds(bool force = false);
		void set_signature(const RootSignature::ptr&);
		void set_pipeline(std::shared_ptr<ComputePipelineState>);


		void dispach(int = 1, int = 1, int = 1);
		void dispach(ivec2, ivec2 = ivec2(8, 8));
		void dispach(ivec3, ivec3 = ivec3(4, 4, 4));




		void execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset = 0, Resource* counter_buffer = nullptr, UINT64 counter_offset = 0);



		void build_ras(const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC &desc)
		{
			get_base().flush_transitions();
			list->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
		}
	
		void dispatch_rays(const D3D12_DISPATCH_RAYS_DESC &desc)
		{
			list->DispatchRays(&desc);
		}

		void set_pso(ComPtr<ID3D12StateObject> pso)
		{
			base.set_pipeline_internal(nullptr);
			list->SetPipelineState1(pso.Get());
		}
	};


	class TransitionCommandList
	{
		ComPtr<ID3D12GraphicsCommandList4> m_commandList;
		ComPtr<ID3D12CommandAllocator> m_commandAllocator;
		CommandListType type;
	public:
		using ptr = std::shared_ptr<TransitionCommandList>;
		inline CommandListType get_type() { return type; }
		TransitionCommandList(CommandListType type);
		void create_transition_list(const std::vector<D3D12_RESOURCE_BARRIER>& transitions, std::vector<Resource*> &duscards);
		ComPtr<ID3D12GraphicsCommandList4> get_native();
	};
}
