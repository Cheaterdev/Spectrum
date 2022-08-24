 module;
#include "helper.h"
export module Graphics:HLSLDescriptors;
//import :Definitions;

import Allocators;
import Math;
//import Resource;

import :Memory;
import :Tiling;
import :Fence;
import :States;
import :Device;
import stl.threading;
import stl.core;

import Log;
import Data;
import d3d12;

import :Descriptors;
import HAL.Types;

import :Resource;
using namespace HAL;
using namespace Graphics;
export
{
	namespace HLSL
	{
		struct RaytracingAccelerationStructure : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;

			RaytracingAccelerationStructure() = default;
			RaytracingAccelerationStructure(const Handle& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource);
		};

		template<class T>
		struct StructuredBuffer : public Handle
		{
			using ptr = int;
			static const HandleType TYPE = HandleType::SRV;

			StructuredBuffer() = default;
			explicit  StructuredBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT first_elem = 0, UINT count = 0);
		};

		template<class T>
		struct RWStructuredBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			RWStructuredBuffer() = default;
			explicit  RWStructuredBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT first_elem = 0, UINT count = 0);
		};

		template<class T>
		struct AppendStructuredBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			AppendStructuredBuffer() = default;
			explicit  AppendStructuredBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* counter_resource, UINT counter_offset, Graphics::Resource* resource, UINT first_elem = 0, UINT count = 0);
		};

		struct ByteAddressBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;

			ByteAddressBuffer() = default;
			explicit ByteAddressBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT offset = 0, UINT size = 0);
		};


		struct RWByteAddressBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			RWByteAddressBuffer() = default;
			explicit RWByteAddressBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT offset = 0, UINT size = 0);
		};

		template<class T>
		struct Buffer : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;

			Buffer() = default;
			explicit  Buffer(const Handle& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, DXGI_FORMAT format, UINT offset = 0, UINT size = 0);
		};

		template<class T>
		struct RWBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			RWBuffer() = default;
			explicit RWBuffer(const Handle& h) : Handle(h)
			{

			}
			void create(Graphics::Resource* resource, DXGI_FORMAT format, UINT offset = 0, UINT size = 0);
		};

		template<class T = float4>
		struct Texture2D : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;


			Texture2D() = default;
			explicit Texture2D(const Handle& h) : Handle(h)
			{

			}

			template<class T2>
			Texture2D(const Texture2D<T2>& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT first_mip = 1, UINT mip_levels = 0, UINT array_offset = 0);
		};

		template<class T = float4>
		struct Texture2DArray : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;

			Texture2DArray() = default;
			explicit Texture2DArray(const Handle& h) : Handle(h)
			{

			}

			template<class T2>
			Texture2DArray(const Texture2DArray<T2>& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT first_mip = 1, UINT mip_levels = 0, UINT array_offset = 0, UINT array_count = 1);
		};

		template<class T = float4>
		struct Texture3D : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;

			Texture3D() = default;
			explicit Texture3D(const Handle& h) : Handle(h)
			{

			}

			template<class T2>
			Texture3D(const Texture3D<T2>& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT first_mip = 1, UINT mip_levels = 0, UINT array_offset = 0);
		};
		template<class T = float4>
		struct TextureCube : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;

			TextureCube() = default;
			explicit TextureCube(const Handle& h) : Handle(h)
			{

			}

			template<class T2>
			TextureCube(const TextureCube<T2>& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT first_mip = 1, UINT mip_levels = 0, UINT array_offset = 0);
		};

		template<class T = float4>
		struct RWTexture2D : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;
			RWTexture2D() = default;
			explicit RWTexture2D(const Handle& h) : Handle(h)
			{

			}

			template<class T2>
			RWTexture2D(const RWTexture2D<T2>& h) : Handle(h)
			{

			}
			void create(Graphics::Resource* resource, UINT first_mip = 1, UINT array_offset = 0);
		};

		template<class T = float4>
		struct RWTexture3D : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			RWTexture3D() = default;
			explicit RWTexture3D(const Handle& h) : Handle(h)
			{

			}
			template<class T2>
			RWTexture3D(const RWTexture3D<T2>& h) : Handle(h)
			{

			}

			void create(Graphics::Resource* resource, UINT first_mip = 1, UINT array_offset = 0);

		};

		template<class T = float4>
		struct RenderTarget : public Handle
		{
			static const HandleType TYPE = HandleType::RTV;

			RenderTarget() = default;
			explicit RenderTarget(const Handle& h) : Handle(h)
			{

			}

			template<class T2>
			RenderTarget(const RenderTarget<T2>& h) : Handle(h)
			{

			}

			void createFrom2D(Graphics::Resource* resource, UINT mip_offset = 0);
			void createFrom2DArray(Graphics::Resource* resource, UINT mip_offset = 0, UINT array_offset = 0);

		};

		template<class T = float4>
		struct DepthStencil : public Handle
		{
			static const HandleType TYPE = HandleType::DSV;

			DepthStencil() = default;
			explicit DepthStencil(const Handle& h) : Handle(h)
			{

			}

			template<class T2>
			DepthStencil(const RenderTarget<T2>& h) : Handle(h)
			{

			}

			void createFrom2D(Graphics::Resource* resource, UINT mip_offset = 0);
			void createFrom2DArray(Graphics::Resource* resource, UINT mip_offset = 0, UINT array_offset = 0);

		};

	}
}
	//	module: private;

		//CD3DX12_RESOURCE_DESC& get_desc(Graphics::Resource* r);
		//D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address(Graphics::Resource* r);
		namespace HLSL
		{
			void RaytracingAccelerationStructure::create(Graphics::Resource* resource)
			{
				D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
				desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
				desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				desc.Format = DXGI_FORMAT_UNKNOWN;
				desc.RaytracingAccelerationStructure.Location = (resource->get_gpu_address());

				Graphics::Device::get().create_srv(*this, nullptr, desc);
			}

			void ByteAddressBuffer::create(Graphics::Resource* resource, UINT offset, UINT size)
			{
				if (size == 0) size = static_cast<UINT>(resource->get_desc().Width / 4);

				D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
				desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				desc.Format = DXGI_FORMAT_R32_TYPELESS;
				desc.Buffer.NumElements = size;
				desc.Buffer.StructureByteStride = 0;
				desc.Buffer.FirstElement = offset;

				desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

				Graphics::Device::get().create_srv(*this, resource, desc);
			}

			void RWByteAddressBuffer::create(Graphics::Resource* resource, UINT offset, UINT size)
			{
				if (size == 0) size = static_cast<UINT>(resource->get_desc().Width / 4);
				D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				desc.Format = DXGI_FORMAT_R32_TYPELESS;
				desc.Buffer.NumElements = size;
				desc.Buffer.StructureByteStride = 0;
				desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
				desc.Buffer.FirstElement = offset;

				Graphics::Device::get().create_uav(*this, resource, desc);
			}



			template<class T>
			void StructuredBuffer<T>::create(Graphics::Resource* resource, UINT first_elem, UINT count)
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

				Graphics::Device::get().create_srv(*this, resource, desc);
			}


			template<class T>
			void RWStructuredBuffer<T>::create(Graphics::Resource* resource, UINT first_elem, UINT count)
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

				Graphics::Device::get().create_uav(*this, resource, desc);
			}


			template<class T>
			void AppendStructuredBuffer<T>::create(Graphics::Resource* counter_resource, UINT counter_offset, Graphics::Resource* resource, UINT first_elem, UINT count)
			{

				D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
				desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

				desc.Buffer.StructureByteStride = sizeof(Underlying<T>);
				desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
				desc.Buffer.NumElements = count;
				desc.Buffer.FirstElement = first_elem;
				desc.Buffer.CounterOffsetInBytes = counter_offset;

				Graphics::Device::get().create_uav(*this, resource, desc, counter_resource);
			}
			template<class T>
			void Buffer<T>::create(Graphics::Resource* resource, DXGI_FORMAT format, UINT first_elem, UINT count)
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

				//Graphics::Device::get().create_srv(*this, resource, desc);
			}


			template<class T>
			void RWBuffer<T>::create(Graphics::Resource* resource, DXGI_FORMAT format, UINT first_elem, UINT count)
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

				Graphics::Device::get().create_uav(*this, resource, desc);
			}




			template<class T>
			void Texture2D<T>::create(Graphics::Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset)
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

				Graphics::Device::get().create_srv(*this, resource, srvDesc);
			}


			template<class T>
			void RWTexture2D<T>::create(Graphics::Resource* resource, UINT first_mip, UINT array_offset)
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


				Graphics::Device::get().create_uav(*this, resource, desc);
			}

			template<class T>
			void Texture2DArray<T>::create(Graphics::Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset, UINT array_count)
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


				Graphics::Device::get().create_srv(*this, resource, srvDesc);
			}



			template<class T>
			void Texture3D<T>::create(Graphics::Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset)
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
				Graphics::Device::get().create_srv(*this, resource, srvDesc);
			}

			template<class T>
			void RWTexture3D<T>::create(Graphics::Resource* resource, UINT first_mip, UINT array_offset)
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
				desc.Format = to_srv(resource->get_desc().Format);
				auto resDesc = resource->get_desc();

				desc.ViewDimension = D3D12_UAV_DIMENSION::D3D12_UAV_DIMENSION_TEXTURE3D;
				desc.Texture3D.FirstWSlice = 0;
				desc.Texture3D.MipSlice = first_mip;
				desc.Texture3D.WSize = -1;// std::max(UINT(1), UINT(resDesc.Depth() >> desc.Texture3D.MipSlice));

				assert(array_offset == 0);

				Graphics::Device::get().create_uav(*this, resource, desc);
			}

			template<class T>
			void TextureCube<T>::create(Graphics::Resource* resource, UINT first_mip, UINT mip_levels, UINT cube_offset)
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


				Graphics::Device::get().create_srv(*this, resource, srvDesc);
			}



			template<class T>
			void RenderTarget<T>::createFrom2D(Graphics::Resource* resource, UINT first_mip)
			{
				D3D12_RENDER_TARGET_VIEW_DESC desc = {};
				auto resDesc = resource->get_desc();

				desc.Format = to_srv(resource->get_desc().Format);


				desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;

				desc.Texture2D.MipSlice = first_mip;
				desc.Texture2D.PlaneSlice = 0;


				Graphics::Device::get().create_rtv(*this, resource, desc);
			}






			template<class T>
			void RenderTarget<T>::createFrom2DArray(Graphics::Resource* resource, UINT first_mip, UINT array_id)
			{
				D3D12_RENDER_TARGET_VIEW_DESC desc = {};
				auto resDesc = resource->get_desc();

				desc.Format = to_srv(resource->get_desc().Format);


				desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2DARRAY;

				desc.Texture2DArray.MipSlice = first_mip;
				desc.Texture2DArray.PlaneSlice = 0;


				desc.Texture2DArray.FirstArraySlice = array_id;
				desc.Texture2DArray.ArraySize = 1;//wtf 

				Graphics::Device::get().create_rtv(*this, resource, desc);

			}





			template<class T>
			void DepthStencil<T>::createFrom2D(Graphics::Resource* resource, UINT first_mip)
			{
				auto resDesc = resource->get_desc();

				D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

				desc.Format = to_dsv(resource->get_desc().Format);


				desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MipSlice = first_mip;

				Graphics::Device::get().create_dsv(*this, resource, desc);
			}






			template<class T>
			void DepthStencil<T>::createFrom2DArray(Graphics::Resource* resource, UINT first_mip, UINT array_id)
			{
				auto resDesc = resource->get_desc();

				D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};

				desc.Format = to_dsv(resource->get_desc().Format);


				desc.ViewDimension = D3D12_DSV_DIMENSION::D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MipSlice = first_mip;
				desc.Texture2DArray.FirstArraySlice = array_id;
				desc.Texture2DArray.ArraySize = 1; // wtf;

				Graphics::Device::get().create_dsv(*this, resource, desc);
			}

		}