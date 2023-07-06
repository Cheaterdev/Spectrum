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


			ComPtr<IDStorageCompressionCodec> g_bufferCompression;

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


			RaytracingPrebuildInfo calculateBuffers(const RaytracingBuildDescBottomInputs& desc);
			RaytracingPrebuildInfo calculateBuffers(const RaytracingBuildDescTopInputs& desc);

		};
	}

}
//module:private;


namespace HAL
{
	namespace API
	{



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
			hr = 	native_device->GetDeviceRemovedReason();
				__debugbreak();
				assert(false);
			}

		}

		//void RaytracingBuildDescBottomInputs::add_geometry(GeometryDesc i)
		//{
		//	D3D12_RAYTRACING_GEOMETRY_DESC geom;
		//	geom.Flags = i.Flags;
		//	geom.Type = i.Type;
		//	geom.Triangles.IndexBuffer = ::to_native(i.IndexBuffer);
		//	geom.Triangles.IndexCount = i.IndexCount;
		//	geom.Triangles.IndexFormat = i.IndexFormat;

		//	geom.Triangles.VertexBuffer.StartAddress = ::to_native(i.VertexBuffer);
		//	geom.Triangles.VertexBuffer.StrideInBytes = i.VertexStrideInBytes;
		//	geom.Triangles.VertexFormat = i.VertexFormat;

		//	geom.Triangles.Transform3x4 = ::to_native(i.Transform3x4);
		//	descs.emplace_back(geom);

		//	geometry.emplace_back(i);
		//}

		//D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS RaytracingBuildDescBottomInputs::to_native() const
		//{
		//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;

		//	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		//	inputs.Flags = ::to_native(Flags);
		//	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
		//	inputs.NumDescs = static_cast<UINT>(descs.size());
		//	inputs.pGeometryDescs = descs.data();

		//	return inputs;
		//}

		//D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS Device::to_native(const RaytracingBuildDescBottomInputs& desc)
		//{
		//	return desc.to_native();
		//}

		//D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS Device::to_native(const RaytracingBuildDescTopInputs& desc)
		//{
		//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;

		//	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		//	inputs.Flags = ::to_native(desc.Flags);
		//	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
		//	inputs.NumDescs = desc.NumDescs;
		//	inputs.InstanceDescs = ::to_native(desc.instances);


		//	return inputs;
		//}

		RaytracingPrebuildInfo Device::calculateBuffers(const RaytracingBuildDescBottomInputs& desc)
		{
			auto inputs = to_native(desc);

			inputs.NumDescs = static_cast<UINT>(inputs.descs.size());
			inputs.pGeometryDescs = inputs.descs.data();


			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

			native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

			return  { info.ResultDataMaxSizeInBytes,info.ScratchDataSizeInBytes,info.UpdateScratchDataSizeInBytes };
		}
		RaytracingPrebuildInfo Device::calculateBuffers(const RaytracingBuildDescTopInputs& desc)
		{
			auto inputs = to_native(desc);
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

			native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
			return { info.ResultDataMaxSizeInBytes,info.ScratchDataSizeInBytes,info.UpdateScratchDataSizeInBytes };
		}


	}
}