module Graphics;
import :CommandList;
import :Types;

namespace HAL
{

	RTXSceneView::RTXSceneView(Resource* resource, FrameResources& frame) :ResourceView(resource)
	{
		/*	init_desc();*/
		scene = to_hal(frame).get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place<HLSL::RaytracingAccelerationStructure>();
		scene.create(resource);
	}



	TextureView TextureView::create_2d_slice(UINT slice, FrameResources& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.ArraySize = 1;
		desc.FirstArraySlice = slice;

		return TextureView(resource, to_hal(frame), desc);
	}

	TextureView TextureView::create_mip(UINT mip, FrameResources& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.MipSlice += mip;
		desc.MipLevels = 1;

		return TextureView(resource, to_hal(frame), desc);
	}
	TextureView CubeView::get_face(UINT face)
	{
		return faces[face];
	}

	CubeView CubeView::create_mip(UINT mip, FrameResources& frame)
	{
		CubeViewDesc desc = view_desc;

		desc.MipSlice += mip;
		desc.MipLevels = 1;

		return CubeView(resource, to_hal(frame), desc);
	}
}