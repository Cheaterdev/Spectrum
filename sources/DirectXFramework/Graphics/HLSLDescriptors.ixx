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
import HAL;

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

			void create(Graphics::Resource* resource, Format format, UINT offset = 0, UINT size = 0);
		};

		template<class T>
		struct RWBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			RWBuffer() = default;
			explicit RWBuffer(const Handle& h) : Handle(h)
			{

			}
			void create(Graphics::Resource* resource, Format format, UINT offset = 0, UINT size = 0);
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

namespace HLSL
{
	void RaytracingAccelerationStructure::create(Graphics::Resource* resource)
	{
		HAL::Views::ShaderResource desc = { resource->get_hal().get(), HAL::Format::UNKNOWN, HAL::Views::ShaderResource::Raytracing {resource->get_resource_address()} };
		Handle::operator=(desc);
	}

	void ByteAddressBuffer::create(Graphics::Resource* resource, UINT offset, UINT size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (size == 0) size = static_cast<UINT>(buffer_desc.SizeInBytes / 4);

		HAL::Views::ShaderResource desc = { resource->get_hal().get(), Format::R32_TYPELESS, HAL::Views::ShaderResource::Buffer {offset, size, 0, true} };
		Handle::operator=(desc);
	}

	void RWByteAddressBuffer::create(Graphics::Resource* resource, UINT offset, UINT size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (size == 0) size = static_cast<UINT>(buffer_desc.SizeInBytes / 4);

		HAL::Views::UnorderedAccess desc = { resource->get_hal().get(), Format::R32_TYPELESS, HAL::Views::UnorderedAccess::Buffer {offset, size, 0, true, 0, nullptr} };
		Handle::operator=(desc);
	}



	template<class T>
	void StructuredBuffer<T>::create(Graphics::Resource* resource, UINT first_elem, UINT count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<UINT>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::ShaderResource desc = { resource->get_hal().get(), Format::UNKNOWN, HAL::Views::ShaderResource::Buffer {first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false} };
		Handle::operator=(desc);
	}


	template<class T>
	void RWStructuredBuffer<T>::create(Graphics::Resource* resource, UINT first_elem, UINT count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<UINT>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::UnorderedAccess desc = { resource->get_hal().get(), Format::UNKNOWN, HAL::Views::UnorderedAccess::Buffer {first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false, 0, nullptr} };
		Handle::operator=(desc);
	}


	template<class T>
	void AppendStructuredBuffer<T>::create(Graphics::Resource* counter_resource, UINT counter_offset, Graphics::Resource* resource, UINT first_elem, UINT count)
	{
		HAL::Views::UnorderedAccess desc = { resource->get_hal().get(), Format::UNKNOWN, HAL::Views::UnorderedAccess::Buffer {first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false, counter_offset, counter_resource->get_hal().get()} };
		Handle::operator=(desc);
	}
	template<class T>
	void Buffer<T>::create(Graphics::Resource* resource, Format format, UINT first_elem, UINT count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<UINT>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::ShaderResource desc = { resource->get_hal().get(), format, HAL::Views::ShaderResource::Buffer {first_elem, static_cast<uint>(count), 0, false} };
		Handle::operator= (desc);
	}


	template<class T>
	void RWBuffer<T>::create(Graphics::Resource* resource, Format format, UINT first_elem, UINT count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<UINT>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));


		HAL::Views::UnorderedAccess desc = { resource->get_hal().get(), format, HAL::Views::UnorderedAccess::Buffer {first_elem, static_cast<uint>(count), 0, false, 0, nullptr} };
		Handle::operator= (desc);
	}




	template<class T>
	void Texture2D<T>::create(Graphics::Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset)
	{
		auto texture_desc = resource->get_desc().as_texture();

		bool is_array = texture_desc.ArraySize > 1;
		if (is_array)
		{
			auto desc = HAL::Views::ShaderResource{
				.Resource = resource->get_hal().get(),
				.Format = texture_desc.Format.to_srv(),
				.View = HAL::Views::ShaderResource::Texture2DArray
					{
						.MostDetailedMip = first_mip,
						.MipLevels = mip_levels,
						.FirstArraySlice = array_offset,
						.ArraySize = 1,
						.PlaneSlice = 0,
						.ResourceMinLODClamp = 0
					}
			};
			Handle::operator= (desc);
		}
		else
		{
			auto desc = HAL::Views::ShaderResource{
				.Resource = resource->get_hal().get(),
				.Format = texture_desc.Format.to_srv(),
				.View = HAL::Views::ShaderResource::Texture2D
					{
						.MostDetailedMip = first_mip,
						.MipLevels = mip_levels,
						.ResourceMinLODClamp = 0
					}
			};
			Handle::operator= (desc);
		}
	}


	template<class T>
	void RWTexture2D<T>::create(Graphics::Resource* resource, UINT first_mip, UINT array_offset)
	{
		auto texture_desc = resource->get_desc().as_texture();
		bool is_array = texture_desc.ArraySize > 1;


		if (!is_array)
		{
			auto desc = HAL::Views::UnorderedAccess{
			.Resource = resource->get_hal().get(),
			.Format = resource->get_desc().as_texture().Format.to_srv(),
			.View = HAL::Views::UnorderedAccess::Texture2D
				{
					.MipSlice = first_mip,
					.PlaneSlice = 0
				}
			};
			Handle::operator= (desc);
		}
		else
		{

			auto desc = HAL::Views::UnorderedAccess{
				.Resource = resource->get_hal().get(),
				.Format = resource->get_desc().as_texture().Format.to_srv(),
				.View = HAL::Views::UnorderedAccess::Texture2DArray
					{
						.MipSlice = first_mip,
						.FirstArraySlice = array_offset,
						.ArraySize = 1,
						.PlaneSlice = 0
					}
			};
			Handle::operator= (desc);
		}
	}

	template<class T>
	void Texture2DArray<T>::create(Graphics::Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset, UINT array_count)
	{
		auto desc = HAL::Views::ShaderResource{
			.Resource = resource->get_hal().get(),
			.Format = resource->get_desc().as_texture().Format.to_srv(),
			.View = HAL::Views::ShaderResource::Texture2DArray
					{
						.MostDetailedMip = first_mip,
						.MipLevels = mip_levels,
						.FirstArraySlice = array_offset,
						.ArraySize = array_count,
						.PlaneSlice = 0,
						.ResourceMinLODClamp = 0
					}
		};
		Handle::operator= (desc);
	}



	template<class T>
	void Texture3D<T>::create(Graphics::Resource* resource, UINT first_mip, UINT mip_levels, UINT array_offset)
	{
		auto desc = HAL::Views::ShaderResource{
				.Resource = resource->get_hal().get(),
				.Format = resource->get_desc().as_texture().Format.to_srv(),
				.View = HAL::Views::ShaderResource::Texture3D
						{
							.MostDetailedMip = first_mip,
							.MipLevels = mip_levels,
							.ResourceMinLODClamp = 0
						}
		};
		Handle::operator= (desc);

		assert(array_offset == 0);
	}

	template<class T>
	void RWTexture3D<T>::create(Graphics::Resource* resource, UINT first_mip, UINT array_offset)
	{
		auto desc = HAL::Views::UnorderedAccess{
				.Resource = resource->get_hal().get(),
				.Format = resource->get_desc().as_texture().Format.to_srv(),
				.View = HAL::Views::UnorderedAccess::Texture3D
						{
							.MipSlice = uint(first_mip),
							.FirstWSlice = 0,
							.WSize = resource->get_desc().as_texture().Dimensions.z >> uint(first_mip)
						}
		};
		Handle::operator= (desc);

		assert(array_offset == 0);
	}

	template<class T>
	void TextureCube<T>::create(Graphics::Resource* resource, UINT first_mip, UINT mip_levels, UINT cube_offset)
	{
		if (cube_offset > 0)
		{
			auto desc = HAL::Views::ShaderResource{
			resource->get_hal().get(),
			resource->get_desc().as_texture().Format.to_srv(),
			HAL::Views::ShaderResource::CubeArray{
				.MostDetailedMip = 0,
				.MipLevels = resource->get_desc().as_texture().MipLevels,
				.First2DArrayFace = cube_offset * 6,
				.NumCubes = 1,
				.ResourceMinLODClamp = 0
			}
			};
			Handle::operator= (desc);

		}
		else
		{

			auto desc = HAL::Views::ShaderResource{
				resource->get_hal().get(),
				resource->get_desc().as_texture().Format.to_srv(),
				HAL::Views::ShaderResource::Cube{
					.MostDetailedMip = 0,
					.MipLevels = resource->get_desc().as_texture().MipLevels,
					.ResourceMinLODClamp = 0
				}
			};
			Handle::operator= (desc);

		}
	}



	template<class T>
	void RenderTarget<T>::createFrom2D(Graphics::Resource* resource, UINT first_mip)
	{
		auto desc = HAL::Views::RenderTarget{
			.Resource = resource->get_hal().get(),
			.Format = resource->get_desc().as_texture().Format.to_srv(),
			.View = HAL::Views::RenderTarget::Texture2D
				{
					.MipSlice = first_mip,
					.PlaneSlice = 0
				}
		};
		Handle::operator= (desc);
	}






	template<class T>
	void RenderTarget<T>::createFrom2DArray(Graphics::Resource* resource, UINT first_mip, UINT array_id)
	{
		auto desc = HAL::Views::RenderTarget{
	.Resource = resource->get_hal().get(),
	.Format = resource->get_desc().as_texture().Format.to_srv(),
	.View = HAL::Views::RenderTarget::Texture2DArray
		{
			.MipSlice = first_mip,
			.FirstArraySlice = uint(array_id),
			.ArraySize = 1,
			.PlaneSlice = 0
		}
		};
		Handle::operator= (desc);


	}





	template<class T>
	void DepthStencil<T>::createFrom2D(Graphics::Resource* resource, UINT first_mip)
	{
		auto desc = HAL::Views::DepthStencil{
		.Resource = resource->get_hal().get(),
		.Format = resource->get_desc().as_texture().Format.to_dsv(),
		.Flags = HAL::Views::DepthStencil::Flags::None,
		.View = HAL::Views::DepthStencil::Texture2D
			{
				.MipSlice = first_mip
			}
		};
		Handle::operator= (desc);
	}






	template<class T>
	void DepthStencil<T>::createFrom2DArray(Graphics::Resource* resource, UINT first_mip, UINT array_id)
	{
		auto desc = HAL::Views::DepthStencil{
		.Resource = resource->get_hal().get(),
		.Format = resource->get_desc().as_texture().Format.to_dsv(),
		.Flags = HAL::Views::DepthStencil::Flags::None,
		.View = HAL::Views::DepthStencil::Texture2DArray
			{
				.MipSlice = first_mip,
				.FirstArraySlice = array_id,
				.ArraySize = 1,
			}
		};
		Handle::operator= (desc);
	}

}