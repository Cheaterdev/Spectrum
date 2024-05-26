module HAL:ResourceViews;

import :Types;

namespace HAL
{

	RTXSceneView::RTXSceneView(const Resource::ptr& resource, GPUEntityStorageInterface& frame) :BufferView(std::static_pointer_cast<Buffer>(resource))
	{
		/*	init_desc();*/
		scene = HLSL::RaytracingAccelerationStructure(frame.alloc_descriptor(1, DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible }));
		
		scene.create(resource);
	}



	Texture2DView Texture2DView::create_2d_slice(UINT slice, GPUEntityStorageInterface& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.ArraySize = 1;
		desc.FirstArraySlice = slice;

		return Texture2DView(resource, frame, desc);
	}

	Texture2DView Texture2DView::create_mip(UINT mip, GPUEntityStorageInterface& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.MipSlice += mip;
		desc.MipLevels = 1;

		return Texture2DView(resource, frame, desc);
	}
	Texture2DView CubeView::get_face(UINT face)
	{
		return faces[face];
	}

	CubeView CubeView::create_mip(UINT mip, GPUEntityStorageInterface& frame)
	{
		CubeViewDesc desc = view_desc;

		desc.MipSlice += mip;
		desc.MipLevels = 1;

		return CubeView(resource, frame, desc);
	}
}
