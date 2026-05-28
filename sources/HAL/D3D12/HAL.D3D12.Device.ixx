export module HAL:API.Device;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;
import Core;

import :Types;
import :Sampler;
import :Utils;
import :Adapter;

using namespace HAL;

export namespace HAL
{

	struct DeviceDesc
	{
		HAL::Adapter::ptr adapter;
	};

	struct DeviceProperties
	{
		std::string name;
		bool rtx = false;
		bool mesh_shader = false;
		bool full_bindless = false;
		bool direct_gpu_upload_heap = false;
		bool work_graph = false;
	};
	namespace API
	{

		class Device
		{
		   std::map<ResourceDesc,ResourceAllocationInfo> alloc_info; 
		protected:
			void init(DeviceDesc& desc);
			virtual ~Device();


		public:
			using ptr = std::shared_ptr<Device>;


			ComPtr<IDStorageCompressionCodec> g_bufferCompression;

		public:
			D3D::Device native_device;
			enum_array<DescriptorHeapType, uint> descriptor_sizes;
			void process_result(HRESULT hr, std::string_view line) const;
		private:
			void dump_dred();
		public:
			uint get_descriptor_size(DescriptorHeapType type) const;
			D3D::Device get_native_device();

			HRESULT get_device_removed_reason() const;

			ResourceAllocationInfo get_alloc_info(const ResourceDesc& desc);
			uint Subresources(const ResourceDesc& desc) const;

			size_t get_vram();


			RaytracingPrebuildInfo calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
			RaytracingPrebuildInfo calculateBuffers(const RaytracingBuildDescTopInputs& desc);

		};
	}

}
