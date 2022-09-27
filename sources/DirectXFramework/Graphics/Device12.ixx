module;

#include "helper.h"


export module Graphics:Device;

import IdGenerator;
import StateContext;
import Data;
import HAL;
import :Definitions;
//using namespace HAL;
export
{

	class GpuCrashTracker;

	namespace Graphics
	{

		struct ResourceAllocationInfo
		{
			size_t size;
			size_t alignment;
			D3D12_HEAP_FLAGS flags;
		};

		typedef D3D12_VIEWPORT Viewport;
		class Device : public Singleton<Device>
		{
			HAL::Device::ptr m_device;
			ComPtr<IDXGIFactory4> factory;
			std::vector<std::shared_ptr<SwapChain>> swap_chains;
			//	std::vector <ComPtr<IDXGIAdapter3> > vAdapters;
			ComPtr<IDXGIAdapter3> adapter;
			//   std::shared_ptr<CommandList> upload_list;
			enum_array<HAL::CommandListType, std::shared_ptr<Queue>> queues;
			IdGenerator<Thread::Lockable> id_generator;
			friend class CommandList;
			bool rtx = false;
			std::unique_ptr<GpuCrashTracker> crasher;

		public:
			void stop_all();
			virtual ~Device();
			void  check_errors();

			UINT get_descriptor_size(HAL::DescriptorHeapType type)
			{
				return m_device->get_descriptor_size(type);
			}
			HAL::Device::ptr get_hal_device()
			{
				return m_device;
			}
			ContextGenerator context_generator;
			std::shared_ptr<CommandList> get_upload_list();
			ComPtr<ID3D12Device5> get_native_device();

			std::shared_ptr<Queue>& get_queue(HAL::CommandListType type);

			size_t get_vram();

			Device();

			ResourceAllocationInfo get_alloc_info(const HAL::ResourceDesc& desc);
			std::shared_ptr<SwapChain> create_swap_chain(const swap_chain_desc& desc);

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescBottomInputs& desc);
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescTopInputs& desc);


			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescTopInputs& desc);

			void create_sampler(D3D12_SAMPLER_DESC desc, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);

			bool is_rtx_supported() { return rtx; }

			mutable bool alive = true;
			void DumpDRED();
		};


	}
}