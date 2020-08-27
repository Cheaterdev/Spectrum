#include "pch.h"

void DX12::RTXSceneView::place_srv(Handle& h) {
	if (!resource) return;

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.RaytracingAccelerationStructure.Location = resource->get_gpu_address();
	
	Device::get().create_srv(h, nullptr, desc);

}


void DX12::BufferView::place_srv(Handle& h) {
	if (!resource) return;

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / 4);
	desc.Buffer.StructureByteStride = 0;
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	Device::get().create_srv(h, resource.get(), desc);

}

void DX12::BufferView::place_uav(Handle& h) {
	if (!resource) return;
	{
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / 4);
		desc.Buffer.StructureByteStride = 0;
		desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

		Device::get().create_uav(h, resource.get(), desc);

		Device::get().create_uav(uav_clear, resource.get(), desc);
	}

}


 void DX12::BufferView::place_cb(Handle& h) {
	if (!resource) return;

	D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
	desc.BufferLocation = resource->get_gpu_address();
	desc.SizeInBytes = view_desc.Buffer.Size;
	assert(desc.SizeInBytes < 65536);

	Device::get().create_cbv(h, resource.get(), desc);
}


void DX12::TextureView::place_rtv(Handle& h) {
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


void DX12::TextureView::place_dsv(Handle& h) {
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


void DX12::TextureView::place_srv(Handle& h) {
	if (!resource) return;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	auto resDesc = resource->get_desc();
	srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);
	srvDesc.Format = to_srv(resource->get_desc().Format);

	//	if (space == PixelSpace::MAKE_LINERAR)
	//		srvDesc.Format = to_linear(srvDesc.Format);


	if (view_desc.type == ResourceType::CUBE)
	{
			int count = resDesc.ArraySize() / 6;

			srvDesc.ViewDimension = count == 1 ? D3D12_SRV_DIMENSION_TEXTURECUBE : D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;

			srvDesc.TextureCube.MipLevels = view_desc.Texture2D.MipLevels;
			srvDesc.TextureCube.MostDetailedMip = view_desc.Texture2D.MipSlice;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0;
	}
	else if (view_desc.type == ResourceType::TEXTURE2D)
	{
			srvDesc.ViewDimension = resDesc.ArraySize() > 1 ? D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2DArray.MipLevels = view_desc.Texture2D.MipLevels;
			srvDesc.Texture2DArray.MostDetailedMip = view_desc.Texture2D.MipSlice;
			srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;

			srvDesc.Texture2DArray.FirstArraySlice = view_desc.Texture2D.FirstArraySlice;
			srvDesc.Texture2DArray.PlaneSlice = 0;
			srvDesc.Texture2DArray.ArraySize = view_desc.Texture2D.ArraySize;
	
	}else if (view_desc.type == ResourceType::TEXTURE3D)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = view_desc.Texture2D.MipLevels;
		srvDesc.Texture3D.MostDetailedMip = view_desc.Texture2D.MipSlice;
		srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
	}
	else assert(false);

	Device::get().create_srv(h, resource.get(), srvDesc);

}

void DX12::TextureView::place_uav(Handle& h) {
	if (!resource) return;

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
	desc.Format = to_srv(resource->get_desc().Format);
	auto resDesc = resource->get_desc();

	if (view_desc.type == ResourceType::TEXTURE2D)
	{
		if (resDesc.ArraySize() == 1)
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
			desc.Texture2D.PlaneSlice = view_desc.Texture2D.PlaneSlice;
			desc.Texture2D.MipSlice = view_desc.Texture2D.MipSlice;
		}
		
		else
		{
			desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.PlaneSlice = view_desc.Texture2D.PlaneSlice;
			desc.Texture2DArray.MipSlice = view_desc.Texture2D.MipSlice;
			desc.Texture2DArray.FirstArraySlice = view_desc.Texture2D.FirstArraySlice;
			desc.Texture2DArray.ArraySize = view_desc.Texture2D.ArraySize == (UINT)(-1) ? 1 : view_desc.Texture2D.ArraySize;
		}

	 

	}else if (view_desc.type == ResourceType::TEXTURE3D)
	{

		desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE3D;
		desc.Texture3D.FirstWSlice = 0;
		desc.Texture3D.MipSlice = view_desc.Texture2D.MipSlice;
		desc.Texture3D.WSize = -1;// std::max(UINT(1), UINT(resDesc.Depth() >> desc.Texture3D.MipSlice));
	}
	else
		return;

	Device::get().create_uav(h, resource.get(), desc);
}

