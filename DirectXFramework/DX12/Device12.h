#pragma once

namespace DX12
{

	class TransitionCommandList;
	class Device;
	class Queue
	{
		std::wstring name;
		ComPtr<ID3D12CommandQueue> native;
		
		ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceEvent;
		UINT64 m_fenceValue;
		std::thread  queue_thread;;
		std::function<void(CommandList*)> del_func;
		std::mutex fence_mutex;
		UINT64 completed_value;



		std::queue<std::shared_ptr<CommandList>> lists;
		std::mutex list_mutex;
		std::queue<std::shared_ptr<CommandList>> tasks;

		std::mutex queue_mutex;
		std::condition_variable condition;

		bool stop = false;
	
		std::mutex ended_mutex;

		std::condition_variable condition_ended;
		CommandListType type;

		using resource_set = std::set<ComPtr<ID3D12Pageable >>;
		std::list<std::pair<UINT64, resource_set>> resources_to_destroy;

		bool is_workload = false;
		UINT64 last_known_fence = 0;
		UINT64 execute_internal(CommandList* list);
	
	public:
		UINT64 signal();

		void unused(ComPtr<ID3D12Pageable> resource);

		std::shared_ptr<CommandList> get_free_list();


		using ptr = std::shared_ptr<Queue>;

		ComPtr<ID3D12CommandQueue> get_native();

		void update_tile_mappings(ID3D12Resource*                  pResource,
			UINT                            NumResourceRegions,
			const D3D12_TILED_RESOURCE_COORDINATE* pResourceRegionStartCoordinates,
			const D3D12_TILE_REGION_SIZE*          pResourceRegionSizes,
			ID3D12Heap*                      pHeap,
			UINT                            NumRanges,
			const D3D12_TILE_RANGE_FLAGS*          pRangeFlags,
			const UINT*                            pHeapRangeStartOffsets,
			const UINT*                            pRangeTileCounts,
			D3D12_TILE_MAPPING_FLAGS        Flags)
		{
			//    std::unique_lock<std::mutex> lock(queue_mutex);
			native->UpdateTileMappings(pResource, NumResourceRegions, pResourceRegionStartCoordinates, pResourceRegionSizes, pHeap, NumRanges, pRangeFlags, pHeapRangeStartOffsets, pRangeTileCounts, Flags);
			//   signal();
		//	wait(signal());
		}

		Queue(CommandListType type, Device * device);

		void stop_all();
		~Queue();
		void wait(UINT64);
		void wait();

		bool is_complete(UINT64 fence);
		UINT64 get_last_step();

		std::shared_future<UINT64> execute(std::shared_ptr<CommandList> list);
		std::shared_future<UINT64> execute(CommandList* list);

	};
	class CommandList;
	class Device : public Singleton<Device>
	{
		ComPtr<ID3D12Device5> m_device;
		ComPtr<IDXGIFactory4> factory;
		std::vector<std::shared_ptr<SwapChain>> swap_chains;


		//   std::shared_ptr<CommandList> upload_list;
		std::array<Queue::ptr, 4> queues;
		IdGenerator id_generator;
		friend class CommandList;
	public:
		void stop_all();
		virtual ~Device();

		void unused(ComPtr<ID3D12Pageable> resource);

		std::shared_ptr<CommandList> get_upload_list();
		ComPtr<ID3D12Device5> get_native_device();

		Queue::ptr& get_queue(CommandListType type);



		Device();

		std::shared_ptr<SwapChain> create_swap_chain(const DX11::swap_chain_desc& desc);

		void create_sampler(D3D12_SAMPLER_DESC desc, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
	};


}