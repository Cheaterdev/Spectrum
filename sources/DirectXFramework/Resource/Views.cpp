#include "pch.h"



 void DX12::BufferView::place_cb(Handle h) {
	if (!resource) return;

	D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
	desc.BufferLocation = resource->get_gpu_address();
	desc.SizeInBytes = (UINT)view_desc.Buffer.Size;
	assert(desc.SizeInBytes < 65536);

	Device::get().create_cbv(h, resource.get(), desc);
}


void DX12::TextureView::place_rtv(Handle h) {
	if (!resource) return;

	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	auto resDesc = resource->get_desc();

	desc.Format = to_srv(resource->get_desc().Format);

	if (view_desc.type == ResourceType::TEXTURE2D)
	{

		if (resDesc.ArraySize() == 1)
		{
			desc.ViewDimension =D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D ;

			desc.Texture2D.MipSlice = view_desc.Texture2D.MipSlice;
			desc.Texture2D.PlaneSlice = view_desc.Texture2D.PlaneSlice;
		}
		else
		{
			desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

			desc.Texture2DArray.MipSlice = view_desc.Texture2D.MipSlice;
			desc.Texture2DArray.PlaneSlice = view_desc.Texture2D.PlaneSlice;


			desc.Texture2DArray.FirstArraySlice = view_desc.Texture2D.FirstArraySlice;
			desc.Texture2DArray.ArraySize = view_desc.Texture2D.ArraySize;
		}
	
	}
	else
		return;

	Device::get().create_rtv(h, resource.get(), desc);
}


void DX12::TextureView::place_dsv(Handle h) {
	if (!resource) return;
	auto resDesc = resource->get_desc();

	D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

	desc.Format = to_dsv(resource->get_desc().Format);


	desc.ViewDimension = resDesc.ArraySize() > 1 ? D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY: D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2DArray.MipSlice = view_desc.Texture2D.MipSlice;
	desc.Texture2DArray.FirstArraySlice = view_desc.Texture2D.FirstArraySlice;
	desc.Texture2DArray.ArraySize = 1;

	Device::get().create_dsv(h, resource.get(), desc);
}

