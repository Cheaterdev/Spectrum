#pragma once

namespace DX12
{
	struct ResourceAllocationInfo
	{
		size_t size;
		size_t alignment;
		D3D12_HEAP_FLAGS flags;
	};
	class Device : public Singleton<Device>
	{
		ComPtr<ID3D12Device5> m_device;
		ComPtr<IDXGIFactory4> factory;
		std::vector<std::shared_ptr<SwapChain>> swap_chains;
		std::vector <ComPtr<IDXGIAdapter3> > vAdapters;

		//   std::shared_ptr<CommandList> upload_list;
		enum_array<CommandListType, Queue::ptr> queues;
		IdGenerator id_generator;
		friend class CommandList;
		bool rtx = false;
	public:
		void stop_all();
		virtual ~Device();

		std::shared_ptr<CommandList> get_upload_list();
		ComPtr<ID3D12Device5> get_native_device();

		Queue::ptr& get_queue(CommandListType type);

		size_t get_vram();

		Device();

		ResourceAllocationInfo get_alloc_info(CD3DX12_RESOURCE_DESC& desc);
		std::shared_ptr<SwapChain> create_swap_chain(const DX12::swap_chain_desc& desc);

		void create_sampler(D3D12_SAMPLER_DESC desc, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);

		bool is_rtx_supported() { return rtx; }
		void create_rtv(Handle h, Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC rtv);
		void create_srv(Handle h, Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC srv);
		void create_uav(Handle h, Resource* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC uav, Resource* counter = nullptr);
		void create_cbv(Handle h, Resource* resource, D3D12_CONSTANT_BUFFER_VIEW_DESC cbv);
		void create_dsv(Handle h, Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC dsv);

	};


}