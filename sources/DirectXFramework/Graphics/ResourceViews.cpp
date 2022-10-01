module Graphics:ResourceViews;
import :CommandList;
import :Types;

namespace Graphics
{

	RTXSceneView::RTXSceneView(Resource* resource, FrameResources& frame) :ResourceView(resource)
	{
		/*	init_desc();*/
		scene = frame.get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place<HLSL::RaytracingAccelerationStructure>();
		scene.create(resource);
	}



	TextureView TextureView::create_2d_slice(UINT slice, FrameResources& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.ArraySize = 1;
		desc.FirstArraySlice = slice;
		desc.cube = false;

		return TextureView(resource, frame, desc);
	}

	TextureView TextureView::create_mip(UINT mip, FrameResources& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.MipSlice += mip;
		desc.MipLevels = 1;

		return TextureView(resource, frame, desc);
	}

}
