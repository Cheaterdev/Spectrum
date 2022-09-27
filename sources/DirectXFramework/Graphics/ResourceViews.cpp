module Graphics:ResourceViews;
import :CommandList;
import :Types;

namespace Graphics
{

	RTXSceneView::RTXSceneView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
	{
		init_desc();
		scene = frame.get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place<HLSL::RaytracingAccelerationStructure>();
		scene.create(resource.get());
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


	void BufferView::place_cb(Handle h) {
		if (!resource) return;

		D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
		desc.BufferLocation = resource->get_gpu_address();
		desc.SizeInBytes = (UINT)view_desc.Buffer.Size;
		assert(desc.SizeInBytes < 65536);
		assert(0);
		//	Device::get().create_cbv(h, resource.get(), desc);
	}

}
