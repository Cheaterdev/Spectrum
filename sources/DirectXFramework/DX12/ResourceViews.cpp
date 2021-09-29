#include "pch.h"
#include "ResourceViews.h"
#include "DX12/CommandList.h"

namespace DX12
{

	RTXSceneView::RTXSceneView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
	{
		init_desc();
		scene = frame.get_cpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place<HLSL::RaytracingAccelerationStructure>();
		scene.create(resource.get());
	}

	BufferView::BufferView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
	{
		init_desc();

		auto& desc = resource->get_desc();
		if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
			rwbyteBuffer = HLSL::RWByteAddressBuffer(frame.get_cpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place());
			rwbyteBuffer.create(resource.get());
		}
	}

}
