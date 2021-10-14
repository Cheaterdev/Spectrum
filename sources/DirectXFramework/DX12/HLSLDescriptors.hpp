#pragma once
#include "Buffer.h"
#include "helper.h"
namespace DX12
{
	namespace HLSL
	{
		template<class T>
		void StructuredBuffer<T>::create(Resource* resource, UINT first_elem, UINT count)
		{

			if (count == 0) count = static_cast<UINT>(resource->get_desc().Width / sizeof(Underlying<T>));
			D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = DXGI_FORMAT_UNKNOWN;

			desc.Buffer.StructureByteStride = sizeof(Underlying<T>);
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
			desc.Buffer.NumElements = count;
			desc.Buffer.FirstElement = first_elem;

			Device::get().create_srv(*this, resource, desc);
		}


		template<class T>
		void RWStructuredBuffer<T>::create(Resource* resource, UINT first_elem, UINT count)
		{
			if (count == 0) count = static_cast<UINT>(resource->get_desc().Width / sizeof(Underlying<T>));

			D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

			desc.Buffer.StructureByteStride = sizeof(Underlying<T>);
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			desc.Buffer.NumElements = count;
			desc.Buffer.FirstElement = first_elem;
			desc.Buffer.CounterOffsetInBytes = 0;

			Device::get().create_uav(*this, resource, desc);
		}


		template<class T>
		void AppendStructuredBuffer<T>::create(Resource* counter_resource, UINT counter_offset, Resource* resource, UINT first_elem, UINT count)
		{

			D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

			desc.Buffer.StructureByteStride = sizeof(Underlying<T>);
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			desc.Buffer.NumElements = count;
			desc.Buffer.FirstElement = first_elem;
			desc.Buffer.CounterOffsetInBytes = counter_offset;

			Device::get().create_uav(*this, resource, desc, counter_resource);
		}
		template<class T>
		void Buffer<T>::create(Resource* resource, DXGI_FORMAT format, UINT first_elem, UINT count)
		{

			if (count == 0) count = static_cast<UINT>(resource->get_desc().Width / sizeof(Underlying<T>));
			D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};

			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = format;
			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			desc.Buffer.NumElements = count;
			desc.Buffer.FirstElement = first_elem;

			Device::get().create_srv(*this, resource, desc);
		}


		template<class T>
		void RWBuffer<T>::create(Resource* resource, DXGI_FORMAT format, UINT first_elem, UINT count)
		{
			if (count == 0) count = static_cast<UINT>(resource->get_desc().Width / sizeof(Underlying<T>));
			D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};

			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

			desc.Format = format;

			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
			desc.Buffer.NumElements = count;
			desc.Buffer.FirstElement = first_elem;
			desc.Buffer.CounterOffsetInBytes = 0;

			Device::get().create_uav(*this, resource, desc);
		}




		template<class T>
		void Texture2D<T>::create(Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			auto resDesc = resource->get_desc();
			srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);
			srvDesc.Format = to_srv(resource->get_desc().Format);

			bool is_array = resDesc.ArraySize() > 1;
			if (is_array)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				srvDesc.Texture2DArray.MipLevels = mip_levels;
				srvDesc.Texture2DArray.MostDetailedMip = first_mip;
				srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;

				srvDesc.Texture2DArray.FirstArraySlice = array_offset;// view_desc.Texture2D.FirstArraySlice;
				srvDesc.Texture2DArray.PlaneSlice = 0;
				srvDesc.Texture2DArray.ArraySize = 1;// view_desc.Texture2D.ArraySize;
			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MipLevels = mip_levels;
				srvDesc.Texture2D.MostDetailedMip = first_mip;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Texture2D.PlaneSlice = 0;
			}

			Device::get().create_srv(*this, resource, srvDesc);
		}


		template<class T>
		void RWTexture2D<T>::create(Resource* resource, UINT first_mip, UINT array_offset)
		{

			D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
			desc.Format = to_srv(resource->get_desc().Format);
			auto resDesc = resource->get_desc();


			bool is_array = resDesc.ArraySize() > 1;


			if (!is_array)
			{
				desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2D;
				desc.Texture2D.PlaneSlice = 0;
				desc.Texture2D.MipSlice = first_mip;
			}
			else
			{
				desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.PlaneSlice = 0;
				desc.Texture2DArray.MipSlice = first_mip;
				desc.Texture2DArray.FirstArraySlice = array_offset;
				desc.Texture2DArray.ArraySize = 1;
			}


			Device::get().create_uav(*this, resource, desc);
		}

		template<class T>
		void Texture2DArray<T>::create(Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset, UINT array_count)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			auto resDesc = resource->get_desc();
			srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);
			srvDesc.Format = to_srv(resource->get_desc().Format);


			srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = mip_levels;
			srvDesc.Texture2DArray.MostDetailedMip = first_mip;
			srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;

			srvDesc.Texture2DArray.FirstArraySlice = array_offset;// view_desc.Texture2D.FirstArraySlice;
			srvDesc.Texture2DArray.PlaneSlice = 0;
			srvDesc.Texture2DArray.ArraySize = array_count;// view_desc.Texture2D.ArraySize;


			Device::get().create_srv(*this, resource, srvDesc);
		}



		template<class T>
		void Texture3D<T>::create(Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			auto resDesc = resource->get_desc();
			srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);
			srvDesc.Format = to_srv(resource->get_desc().Format);

			srvDesc.ViewDimension = D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE3D;
			srvDesc.Texture3D.MipLevels = mip_levels;
			srvDesc.Texture3D.MostDetailedMip = first_mip;
			srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;

			assert(array_offset == 0);
			Device::get().create_srv(*this, resource, srvDesc);
		}

		template<class T>
		void RWTexture3D<T>::create(Resource* resource, UINT first_mip, UINT array_offset)
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
			desc.Format = to_srv(resource->get_desc().Format);
			auto resDesc = resource->get_desc();

			desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE3D;
			desc.Texture3D.FirstWSlice = 0;
			desc.Texture3D.MipSlice = first_mip;
			desc.Texture3D.WSize = -1;// std::max(UINT(1), UINT(resDesc.Depth() >> desc.Texture3D.MipSlice));

			assert(array_offset == 0);

			Device::get().create_uav(*this, resource, desc);
		}

		template<class T>
		void TextureCube<T>::create(Resource* resource, UINT first_mip, UINT mip_levels, UINT cube_offset)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			auto resDesc = resource->get_desc();
			srvDesc.Shader4ComponentMapping = get_default_mapping(resource->get_desc().Format);
			srvDesc.Format = to_srv(resource->get_desc().Format);

			if (cube_offset > 0)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;

				srvDesc.TextureCubeArray.MipLevels = mip_levels;
				srvDesc.TextureCubeArray.MostDetailedMip = first_mip;
				srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.0;
				srvDesc.TextureCubeArray.First2DArrayFace = cube_offset * 6;

			}
			else
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

				srvDesc.TextureCube.MipLevels = mip_levels;
				srvDesc.TextureCube.MostDetailedMip = first_mip;
				srvDesc.TextureCube.ResourceMinLODClamp = 0.0;
			}


			Device::get().create_srv(*this, resource, srvDesc);
		}



		template<class T>
		void RenderTarget<T>::createFrom2D(Resource* resource, UINT first_mip)
		{
			D3D12_RENDER_TARGET_VIEW_DESC desc = {};
			auto resDesc = resource->get_desc();

			desc.Format = to_srv(resource->get_desc().Format);


			desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;

			desc.Texture2D.MipSlice = first_mip;
			desc.Texture2D.PlaneSlice = 0;


			Device::get().create_rtv(*this, resource, desc);
		}






		template<class T>
		void RenderTarget<T>::createFrom2DArray(Resource* resource, UINT first_mip, UINT array_id)
		{
			D3D12_RENDER_TARGET_VIEW_DESC desc = {};
			auto resDesc = resource->get_desc();

			desc.Format = to_srv(resource->get_desc().Format);


			desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

			desc.Texture2DArray.MipSlice = first_mip;
			desc.Texture2DArray.PlaneSlice = 0;


			desc.Texture2DArray.FirstArraySlice = array_id;
			desc.Texture2DArray.ArraySize = 1;//wtf 

			Device::get().create_rtv(*this, resource, desc);

		}





		template<class T>
		void DepthStencil<T>::createFrom2D(Resource* resource, UINT first_mip)
		{
			auto resDesc = resource->get_desc();

			D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

			desc.Format = to_dsv(resource->get_desc().Format);


			desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = first_mip;

			Device::get().create_dsv(*this, resource, desc);
		}






		template<class T>
		void DepthStencil<T>::createFrom2DArray(Resource* resource, UINT first_mip, UINT array_id)
		{
			auto resDesc = resource->get_desc();

			D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

			desc.Format = to_dsv(resource->get_desc().Format);


			desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			desc.Texture2DArray.MipSlice = first_mip;
			desc.Texture2DArray.FirstArraySlice = array_id;
			desc.Texture2DArray.ArraySize = 1; // wtf;

			Device::get().create_dsv(*this, resource, desc);
		}

	}

}
