#include "pch.h"

void DX12::BufferView::place_srv(Handle& h) {
	if (!resource) return;

	D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / 4);
	desc.Buffer.StructureByteStride = 0;
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	/*
	if (srv.Buffer.StructureByteStride > 0) {
		srv.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / srv.Buffer.StructureByteStride);
		srv.Buffer.FirstElement =  view_desc.Buffer.Offset / srv.Buffer.StructureByteStride;

	}
	else {
		srv.Buffer.NumElements = 0;
		srv.Buffer.FirstElement = 0;

	}*/
	
	Device::get().get_native_device()->CreateShaderResourceView(resource->get_native().Get(), &desc, h.cpu);
}


void DX12::TextureView::place_rtv(Handle& h) {
	if (!resource) return;

	D3D12_RENDER_TARGET_VIEW_DESC desc = {};

	desc.Format = to_srv(resource->get_desc().Format);

	if (view_desc.type == ResourceType::TEXTURE2D)
	{
		desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = view_desc.Texture2D.MipSlice;
		desc.Texture2D.PlaneSlice = view_desc.Texture2D.FirstArraySlice;
	}


	Device::get().get_native_device()->CreateRenderTargetView(resource->get_native().Get(), &desc, h.cpu);

}


void DX12::TextureView::place_dsv(Handle& h) {
	if (!resource) return;
	auto resDesc = resource->get_desc();

	D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

	desc.Format = to_dsv(resource->get_desc().Format);


	desc.ViewDimension = resDesc.ArraySize() > 1 ? D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY: D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Texture2DArray.MipSlice = view_desc.Texture2D.MipSlice;
	desc.Texture2DArray.FirstArraySlice = view_desc.Texture2D.FirstArraySlice;
	desc.Texture2DArray.ArraySize = 1;

	Device::get().get_native_device()->CreateDepthStencilView(resource->get_native().Get(), &desc, h.cpu);
}


void DX12::TextureView::place_srv(Handle& h) {
	if (!resource) return;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	auto resDesc = resource->get_desc();
	srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);
	srvDesc.Format = to_srv(resource->get_desc().Format);

	//	if (space == PixelSpace::MAKE_LINERAR)
	//		srvDesc.Format = to_linear(srvDesc.Format);

	if (view_desc.type == ResourceType::TEXTURE2D)
	{
		srvDesc.ViewDimension = resDesc.ArraySize()>1 ? D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2DArray.MipLevels = view_desc.Texture2D.MipLevels;
		srvDesc.Texture2DArray.MostDetailedMip = view_desc.Texture2D.MipSlice;
		srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;

		srvDesc.Texture2DArray.FirstArraySlice = view_desc.Texture2D.FirstArraySlice;
		srvDesc.Texture2DArray.PlaneSlice = 0;
		srvDesc.Texture2DArray.ArraySize = view_desc.Texture2D.ArraySize;
	}


	Device::get().get_native_device()->CreateShaderResourceView(resource->get_native().Get(), &srvDesc, h.cpu);

}

void DX12::TextureView::place_uav(Handle& h) {
	if (!resource) return;

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = resource->get_desc().Format;
	auto resDesc = resource->get_desc();

	if (view_desc.type == ResourceType::TEXTURE2D)
	{
		desc.ViewDimension = resDesc.ArraySize() > 1 ?  D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2DARRAY : D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
		desc.Texture2D.PlaneSlice = view_desc.Texture2D.PlaneSlice;
		desc.Texture2D.MipSlice = view_desc.Texture2D.MipSlice;

		desc.Texture2DArray.PlaneSlice = view_desc.Texture2D.PlaneSlice;
		desc.Texture2DArray.MipSlice = view_desc.Texture2D.MipSlice;
		desc.Texture2DArray.FirstArraySlice = view_desc.Texture2D.FirstArraySlice;
		desc.Texture2DArray.ArraySize = view_desc.Texture2D.ArraySize==(UINT)(-1)?1: view_desc.Texture2D.ArraySize;
	}


	Device::get().get_native_device()->CreateUnorderedAccessView(resource->get_native().Get(), nullptr, &desc, h.cpu);
}

