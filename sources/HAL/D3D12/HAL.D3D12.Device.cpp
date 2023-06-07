module HAL:Device;
import :Debug;
import :Utils;

import d3d12; 
import Core;

#include "GFSDK_Aftermath.h"
#include "NsightAftermathGpuCrashTracker.h"

#undef THIS
namespace HAL
{


	
texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource)
{
	auto & desc = rdesc.as_texture();

	UINT64 RequiredSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
	UINT NumRows;
	UINT64 RowSizesInBytes;
	D3D::ResourceDesc Desc = ::to_native(rdesc);
	HAL::Device::get().get_native_device()->GetCopyableFootprints1(&Desc, sub_resource, 1, 0, &Layouts, &NumRows, &RowSizesInBytes, &RequiredSize);

	return { RequiredSize , NumRows , Layouts.Footprint.RowPitch , static_cast<uint>(NumRows * Layouts.Footprint.RowPitch),D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, from_native(Layouts.Footprint.Format) };
}



texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource, ivec3 box)
{
	auto & desc = rdesc.as_texture();

	UINT rows_count = box.y;

	if (desc.Format ==Format::BC7_UNORM_SRGB || desc.Format == Format::BC7_UNORM)
		rows_count /= 4;
	UINT64 RequiredSize = 0;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
	UINT NumRows;
	UINT64 RowSizesInBytes;
	D3D::ResourceDesc Desc = ::to_native(rdesc);
	HAL::Device::get().get_native_device()->GetCopyableFootprints1(&Desc, sub_resource, 1, 0, &Layouts, &NumRows, &RowSizesInBytes, &RequiredSize);
	UINT64 res_stride = Math::AlignUp(RowSizesInBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	UINT64 size = res_stride * rows_count * box.z;

	return { size , rows_count , static_cast<uint>(res_stride) , static_cast<uint>(res_stride * rows_count),D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, from_native(Layouts.Footprint.Format) };
}


		std::vector<std::byte> Device::compress(std::span<std::byte> source)
		{
			std::vector<std::byte> dest;


			size_t maxSize = g_bufferCompression->CompressBufferBound(static_cast<uint32_t>(source.size()));

			dest.resize(maxSize);

			size_t actualCompressedSize = 0;

			HRESULT compressionResult = g_bufferCompression->CompressBuffer(
				reinterpret_cast<const void*>(source.data()),
				static_cast<uint32_t>(source.size()),
				DSTORAGE_COMPRESSION_BEST_RATIO,
				reinterpret_cast<void*>(dest.data()),
				static_cast<uint32_t>(dest.size()),
				&actualCompressedSize);

			dest.resize(actualCompressedSize);
			return dest;
		}
	namespace API
	{
		
		size_t Device::get_vram()
		{
			auto THIS = static_cast<HAL::Device*>(this);

			DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
			THIS->adapter->native_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
			size_t usedVRAM = videoMemoryInfo.CurrentUsage / 1024 / 1024;
			return usedVRAM;
		}

		void Device::init(DeviceDesc& desc)
		{
			auto THIS = static_cast<HAL::Device*>(this);

			D3D12CreateDevice(
				desc.adapter->native_adapter.Get(),
				D3D_FEATURE_LEVEL_12_1,
				IID_PPV_ARGS(&native_device)
			);

			if (!native_device) return;

			THIS->adapter = desc.adapter;

			for (auto type : magic_enum::enum_values<DescriptorHeapType>())
			{
					descriptor_sizes[type] = native_device->GetDescriptorHandleIncrementSize(::to_native(type));
			}


			D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
			D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};
			D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_7 };

			TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(options12)));
			TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)));
			TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
			TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
			TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel)));

			auto& properties = THIS->properties;
			properties.rtx = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
			properties.full_bindless = supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6;
			properties.mesh_shader = options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER::D3D12_MESH_SHADER_TIER_1;


			if constexpr (HAL::Debug::ValidationErrors)
			{
				ComPtr<ID3D12InfoQueue> d3dInfoQueue;
				if (SUCCEEDED(native_device.As(&d3dInfoQueue)))
				{

					//	d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
					//	d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
					D3D12_MESSAGE_ID hide[] =
					{
						D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_INTERSECTS_MULTIPLE_BUFFERS,
						D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_GETHEAPPROPERTIES_INVALIDRESOURCE,
						D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT,
						D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_NON_RETAIL_SHADER_MODEL_WONT_VALIDATE,
						D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_CREATEPIPELINESTATE_CACHEDBLOBDESCMISMATCH,
						D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_EMPTY_DISPATCH,

						D3D12_MESSAGE_ID(1380)//D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT_SMALLRESOURCE
					};

					D3D12_INFO_QUEUE_FILTER filter = {};
					filter.DenyList.NumIDs = _countof(hide);
					filter.DenyList.pIDList = hide;
					d3dInfoQueue->AddStorageFilterEntries(&filter);
				}
			}


			const uint32_t aftermathFlags =
				GFSDK_Aftermath_FeatureFlags_EnableMarkers |             // Enable event marker tracking.
				GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |    // Enable tracking of resources.
				GFSDK_Aftermath_FeatureFlags_CallStackCapturing;    // Generate debug information for shaders.

			auto afterres = GFSDK_Aftermath_DX12_Initialize(
				GFSDK_Aftermath_Version_API,
				aftermathFlags,
				native_device.Get());



			  DStorageCreateCompressionCodec(
            DSTORAGE_COMPRESSION_FORMAT_GDEFLATE,
            6,
            IID_PPV_ARGS(&g_bufferCompression));

		}

		D3D::Device Device::get_native_device()
		{
			return native_device;
		}

		ResourceAllocationInfo Device::get_alloc_info(const ResourceDesc& desc)
		{
			auto native_desc = ::to_native(desc);
			if (native_desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
			{

				if ((native_desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
				{
					native_desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
				}
			}
			if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
			{
				assert(native_desc.SampleDesc.Count > 0);
			}


			D3D12_RESOURCE_ALLOCATION_INFO1 info2;
			
			auto info = native_device->GetResourceAllocationInfo2(0, 1, &native_desc, &info2);
			if(info.SizeInBytes==UINT64_MAX)
			{
				native_desc.Alignment = 0;
				 info = native_device->GetResourceAllocationInfo2(0, 1, &native_desc, &info2);
			}
			assert(info.SizeInBytes!=UINT64_MAX);


			native_desc.Alignment = info.Alignment;


			// TODO small alignment
		/*	if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
			{
				native_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				info = native_device->GetResourceAllocationInfo(0, 1, &native_desc);


				if (info.Alignment != D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
				{
					native_desc.Alignment = 0;
					info = native_device->GetResourceAllocationInfo(0, 1, &native_desc);
				}

			}*/

			ResourceAllocationInfo result;

			result.size = info.SizeInBytes;
			result.alignment = info.Alignment;
			result.flags = HeapFlags::NONE;


			if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				result.flags |= HeapFlags::BUFFERS_ONLY;
			}
			else if (native_desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
			{
				result.flags |= HeapFlags::RTDS_ONLY;
			}
			else
				result.flags |= HeapFlags::TEXTURES_ONLY;
			if constexpr (Debug::CheckErrors)	TEST(*this, native_device->GetDeviceRemovedReason());

			assert(result.size != UINT64_MAX);
			return result;
		}


	}
}