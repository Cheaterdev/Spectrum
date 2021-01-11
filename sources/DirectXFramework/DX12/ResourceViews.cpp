#include "pch.h"
#include "ResourceViews.h"
#include "CommandList.h"

namespace DX12
{
/*	void ResourceView::init_views(FrameResources& frame) {

		auto& desc = resource->get_desc();

		if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
		{
			rtv = frame.rtv_cpu.place();
			place_rtv(rtv);
		}
		if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
			dsv = frame.dsv_cpu.place();
			place_dsv(dsv);
		}
		if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
			uav = frame.srv_uav_cbv_cpu.place();
			uav_clear = frame.srv_uav_cbv_cpu.place();
			place_uav(uav);
		}

		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && desc.Flags == D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE && desc.Width < 65536) {
			cb = frame.srv_uav_cbv_cpu.place();
			place_cb(cb);
		}

		if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
			srv = frame.srv_uav_cbv_cpu.place();
			place_srv(srv);
		}
	}*/

	RTXSceneView::RTXSceneView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
	{
		init_desc();
		//	init_views(frame);
		srv_handle = frame.get_cpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place<HLSL::RaytracingAccelerationStructure>();
		place_srv(srv_handle);
	}

	BufferView::BufferView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
	{
		init_desc();
		init_views(frame);

		auto& desc = resource->get_desc();
		if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
			uav_handle = HLSL::RWByteAddressBuffer(frame.get_cpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place());
			place_uav(uav_handle);
		}
	}

}
