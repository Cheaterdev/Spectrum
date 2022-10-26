export module HAL:API.Device;
import d3d12;
import Math;
import Utils;

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
	};
	namespace API
	{

		class Device
		{

		protected:
			void init(DeviceDesc& desc);
			virtual ~Device() = default;


		public:
			using ptr = std::shared_ptr<Device>;




		public:
			D3D::Device native_device;
			enum_array<DescriptorHeapType, uint> descriptor_sizes;
			void process_result(HRESULT hr, std::string_view line) const;
		private:
			void dump_dred();
		public:
			uint get_descriptor_size(DescriptorHeapType type) const
			{
				return descriptor_sizes[type];

			}
			D3D::Device get_native_device();

			ResourceAllocationInfo get_alloc_info(const ResourceDesc& desc);
			uint Subresources(const ResourceDesc& desc) const
			{
				if (desc.is_buffer())
					return 1;

				auto texture_desc = desc.as_texture();
				uint count = D3D12GetFormatPlaneCount(native_device.Get(), ::to_native(texture_desc.Format));
				return texture_desc.MipLevels * texture_desc.ArraySize * count;
			}

			size_t get_vram();


/// todo
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescBottomInputs& desc);
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS to_native(const RaytracingBuildDescTopInputs& desc);


			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO calculateBuffers(const RaytracingBuildDescTopInputs& desc);

		};
	}

}
//module:private;


namespace HAL
{
	namespace API
	{


		size_t Device::get_vram()
		{
			DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
			//	adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
			size_t usedVRAM = videoMemoryInfo.CurrentUsage / 1024 / 1024;
			return usedVRAM;
		}

		void Device::dump_dred()
		{
			ComPtr<ID3D12DeviceRemovedExtendedData>  pDred;
			TEST(*this, native_device->QueryInterface(IID_PPV_ARGS(&pDred)));

			D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput = {};
			D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput = {};
			TEST(*this, pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
			TEST(*this, pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));

			auto parse_node = [](const D3D12_AUTO_BREADCRUMB_NODE& node)
			{
				Log::get() << node << Log::endl;
			};


			auto node = DredAutoBreadcrumbsOutput.pHeadAutoBreadcrumbNode;

			while (node)
			{
				parse_node(*node);
				node = node->pNext;
			}

		}



		void Device::process_result(HRESULT hr, std::string_view line) const
		{
			//if (hr == 0x887a0005) device_fail();

			std::string message = std::system_category().message(hr);

			if (FAILED(hr))
				Log::get().crash_error(hr, line);

			if (FAILED(hr))
			{
				__debugbreak();
				assert(false);
			}

		}



		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS Device::to_native(const RaytracingBuildDescBottomInputs& desc)
		{
			return desc.to_native();
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS Device::to_native(const RaytracingBuildDescTopInputs& desc)
		{
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;

			inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			inputs.Flags = ::to_native(desc.Flags);
			inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.NumDescs = desc.NumDescs;
			inputs.InstanceDescs = ::to_native(desc.instances);


			return inputs;
		}

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO Device::calculateBuffers(const RaytracingBuildDescBottomInputs& desc)
		{
			auto inputs = to_native(desc);
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

			native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

			return info;
		}
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO Device::calculateBuffers(const RaytracingBuildDescTopInputs& desc)
		{
			auto inputs = to_native(desc);
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

			native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
			return info;
		}


	}
}