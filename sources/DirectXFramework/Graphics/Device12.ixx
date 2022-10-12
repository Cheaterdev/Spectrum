export module Graphics:Device;

import IdGenerator;
import Singleton;
import StateContext;
import Data;
import HAL;
import :Definitions;

import d3d12;
//using namespace HAL;
export
{

	

	namespace Graphics
	{

		struct ResourceAllocationInfo
		{
			size_t size;
			size_t alignment;
			D3D12_HEAP_FLAGS flags;
		};

		typedef D3D12_VIEWPORT Viewport;
		class Device : public Singleton<Device>, public HAL::Device
		{
			friend class HAL::Device;
			friend class Singleton<Device>;
		private:
			
			HAL::DeviceProperties properties;
			std::vector<std::shared_ptr<SwapChain>> swap_chains;
			HAL::Adapter::ptr adapter;
			enum_array<HAL::CommandListType, std::shared_ptr<Queue>> queues;
			IdGenerator<Thread::Lockable> id_generator;
			friend class CommandList;
			bool rtx = false;
		
		public:
			Device(HAL::DeviceDesc desc);
			virtual ~Device();

			static std::shared_ptr<Device> create_singleton();

			void stop_all();
		
			const HAL::DeviceProperties& get_properties() const { return properties;}
			ContextGenerator context_generator;
			std::shared_ptr<CommandList> get_upload_list();
			ComPtr<ID3D12Device5> get_native_device();

			std::shared_ptr<Queue>& get_queue(HAL::CommandListType type);

	
		
			ResourceAllocationInfo get_alloc_info(const HAL::ResourceDesc& desc);
			std::shared_ptr<SwapChain> create_swap_chain(const swap_chain_desc& desc);

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescBottomInputs& desc);
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescTopInputs& desc);


			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescTopInputs& desc);

			bool is_rtx_supported() { return rtx; }

			mutable bool alive = true;
		};

	}
}