#include "pch_dx.h"
import ResourceViews;
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


	TextureView TextureView::create_2d_slice(UINT slice, FrameResources& frame)
	{
		ResourceViewDesc desc = view_desc;
		desc.type = ResourceType::TEXTURE2D;
		desc.Texture2D.ArraySize = 1;
		desc.Texture2D.FirstArraySlice = slice;

		return TextureView(resource, frame, desc);
	}

	TextureView TextureView::create_mip(UINT mip, FrameResources& frame)
	{
		ResourceViewDesc desc = view_desc;

		desc.Texture2D.MipSlice += mip;
		desc.Texture2D.MipLevels = 1;

		return TextureView(resource, frame, desc);
	}
}
