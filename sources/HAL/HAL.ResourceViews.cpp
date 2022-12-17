module HAL:ResourceViews;

import :Types;

namespace HAL
{

	RTXSceneView::RTXSceneView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
	{
		/*	init_desc();*/
		scene = HLSL::RaytracingAccelerationStructure(frame.alloc_descriptor(1, DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible }));
		
		scene.create(resource);
	}



	TextureView TextureView::create_2d_slice(UINT slice, FrameResources& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.ArraySize = 1;
		desc.FirstArraySlice = slice;

		return TextureView(resource, frame, desc);
	}

	TextureView TextureView::create_mip(UINT mip, FrameResources& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.MipSlice += mip;
		desc.MipLevels = 1;

		return TextureView(resource, frame, desc);
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

		return CubeView(resource, frame, desc);
	}
}
