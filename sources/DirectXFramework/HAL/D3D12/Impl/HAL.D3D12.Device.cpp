module Graphics:Device;

import HAL;
import d3d12;
import Math;
import Utils;
import Debug;

#include "GFSDK_Aftermath.h"
#include "NsightAftermathGpuCrashTracker.h"

#undef THIS
namespace HAL
{
	void Device::init(DeviceDesc& desc)
	{
		auto THIS = static_cast<Graphics::Device*>(this);

		D3D12CreateDevice(
			desc.adapter->native_adapter.Get(),
			D3D_FEATURE_LEVEL_12_1,
			IID_PPV_ARGS(&native_device)
		);

		if (!native_device) return;

		for (auto type : magic_enum::enum_values<DescriptorHeapType>())
		{
			if (type != DescriptorHeapType::__GENERATE_OPS__) // TODO: ooooooooooooooooooooops
				descriptor_sizes[type] = native_device->GetDescriptorHandleIncrementSize(to_native(type));
		}


		D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
		D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_7 };

		TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)));
		TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
		TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
		TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel)));

		auto &properties = THIS->properties;
		properties.rtx = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
		properties.full_bindless = supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6;
		properties.mesh_shader = options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER::D3D12_MESH_SHADER_TIER_1;


		if constexpr(HAL::Debug::ValidationErrors)
		{
			ComPtr<ID3D12InfoQueue> d3dInfoQueue;
			if (SUCCEEDED(native_device.As(&d3dInfoQueue)))
			{

				//	d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
				//	d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
				D3D12_MESSAGE_ID hide[] =
				{
					D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_INTERSECTS_MULTIPLE_BUFFERS,
					D3D12_MESSAGE_ID_GETHEAPPROPERTIES_INVALIDRESOURCE,
					D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT,
					D3D12_MESSAGE_ID_NON_RETAIL_SHADER_MODEL_WONT_VALIDATE,
					D3D12_MESSAGE_ID_CREATEPIPELINESTATE_CACHEDBLOBDESCMISMATCH,
					D3D12_MESSAGE_ID_EMPTY_DISPATCH
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


	}


	ResourceAllocationInfo Device::get_alloc_info(const ResourceDesc& desc)
	{
		auto native_desc = ::to_native(desc);
		/*if (native_desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{

			if ((native_desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
			{
				native_desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			}
		}*/
		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{

			assert(native_desc.SampleDesc.Count > 0);
		}
		D3D12_RESOURCE_ALLOCATION_INFO info = native_device->GetResourceAllocationInfo(0, 1, &native_desc);
		native_desc.Alignment = info.Alignment;


		// TODO small alignment
		/*if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
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
		if constexpr (BuildOptions::Debug)	TEST(*this, native_device->GetDeviceRemovedReason());

		assert(result.size != UINT64_MAX);
		return result;
	}


}