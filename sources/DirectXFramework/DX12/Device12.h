#pragma once

#include "utils/utils_macros.h"
#include "helper.h"
import IdGenerator;
import StateContext;
import Data;
import d3d12_types;
class GpuCrashTracker;

namespace DX12
{
	struct swap_chain_desc;
	struct RaytracingBuildDescBottomInputs;
	struct RaytracingBuildDescTopInputs;
	class SwapChain;
	class Queue;
	struct Handle;
	class Resource;
	struct ResourceAllocationInfo;

	typedef D3D12_VIEWPORT Viewport;
	class CommandList;

	class Device : public Singleton<Device>
	{
		ComPtr<ID3D12Device5> m_device;
		ComPtr<IDXGIFactory4> factory;
		std::vector<std::shared_ptr<SwapChain>> swap_chains;
		std::vector <ComPtr<IDXGIAdapter3> > vAdapters;

		//   std::shared_ptr<CommandList> upload_list;
		enum_array<CommandListType, std::shared_ptr<Queue>> queues;
		IdGenerator<Thread::Lockable> id_generator;
		friend class CommandList;
		bool rtx = false;
		std::unique_ptr<GpuCrashTracker> crasher;
	public:
		void stop_all();
		virtual ~Device();
		void  check_errors();
		ContextGenerator context_generator;
		std::shared_ptr<CommandList> get_upload_list();
		ComPtr<ID3D12Device5> get_native_device();

		std::shared_ptr<Queue>& get_queue(CommandListType type);

		size_t get_vram();

		Device();

		ResourceAllocationInfo get_alloc_info(CD3DX12_RESOURCE_DESC& desc);
		std::shared_ptr<SwapChain> create_swap_chain(const swap_chain_desc& desc);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescBottomInputs& desc);
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescTopInputs& desc);
		
			
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescTopInputs& desc);

		void create_sampler(D3D12_SAMPLER_DESC desc, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);

		bool is_rtx_supported() { return rtx; }
		void create_rtv(Handle h, Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC rtv);
		void create_srv(Handle h, Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC srv);
		void create_uav(Handle h, Resource* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC uav, Resource* counter = nullptr);
		void create_cbv(Handle h, Resource* resource, D3D12_CONSTANT_BUFFER_VIEW_DESC cbv);
		void create_dsv(Handle h, Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC dsv);

		mutable bool alive = true;
		void DumpDRED();
	};


}