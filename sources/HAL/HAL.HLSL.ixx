export module HAL:HLSL;

import :Descriptors;
import :DescriptorHeap;

import :Resource;
import :Concepts;

using namespace HAL;

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

			void create(Resource::ptr resource);
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

			void create(Resource::ptr resource, uint64 first_elem = 0, uint64 count = 0);
		};

		template<class T>
		struct ConstantBuffer : public Handle
		{
			using ptr = int;
			static const HandleType TYPE = HandleType::CBV;

			ConstantBuffer() = default;
			explicit  ConstantBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Resource::ptr resource,uint64 offset, uint64 size);
		};

		template<class T>
		struct RWStructuredBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			RWStructuredBuffer() = default;
			explicit  RWStructuredBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Resource::ptr resource, uint64 first_elem = 0, uint64 count = 0);
		};

		template<class T>
		struct AppendStructuredBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			AppendStructuredBuffer() = default;
			explicit  AppendStructuredBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Resource::ptr counter_resource, uint64 counter_offset, Resource::ptr resource, uint64 first_elem = 0, uint64 count = 0);
		};

		struct ByteAddressBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;

			ByteAddressBuffer() = default;
			explicit ByteAddressBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Resource::ptr resource, uint64 offset = 0, uint64 size = 0);
		};


		struct RWByteAddressBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			RWByteAddressBuffer() = default;
			explicit RWByteAddressBuffer(const Handle& h) : Handle(h)
			{

			}

			void create(Resource::ptr resource, uint64 offset = 0, uint64 size = 0);
		};

		template<class T>
		struct Buffer : public Handle
		{
			static const HandleType TYPE = HandleType::SRV;

			Buffer() = default;
			explicit  Buffer(const Handle& h) : Handle(h)
			{

			}

			void create(Resource::ptr resource, Format format, uint64 offset = 0, uint64 size = 0);
		};

		template<class T>
		struct RWBuffer : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;

			RWBuffer() = default;
			explicit RWBuffer(const Handle& h) : Handle(h)
			{

			}
			void create(Resource::ptr resource, Format format, uint64 offset = 0, uint64 size = 0);
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

			void create(Resource::ptr resource, uint first_mip = 1, uint mip_levels = 0, uint array_offset = 0);
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

			void create(Resource::ptr resource, uint first_mip = 1, uint mip_levels = 0, uint array_offset = 0, uint array_count = 1);
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

			void create(Resource::ptr resource, uint first_mip = 1, uint mip_levels = 0);
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

			void create(Resource::ptr resource, uint first_mip = 1, uint mip_levels = 0, uint array_offset = 0);
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
			void create(Resource::ptr resource, uint first_mip = 1, uint array_offset = 0);
		};

		template<class T = float4>
		struct RWTexture2DArray : public Handle
		{
			static const HandleType TYPE = HandleType::UAV;
			RWTexture2DArray() = default;
			explicit RWTexture2DArray(const Handle& h) : Handle(h)
			{

			}

			template<class T2>
			RWTexture2DArray(const RWTexture2DArray<T2>& h) : Handle(h)
			{

			}
			void create(Resource::ptr resource, uint first_mip = 0, uint mip_levels = 1, uint array_offset = 0, uint array_count = 1);
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

			void create(Resource::ptr resource, uint first_mip = 1);

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

			void createFrom2D(Resource::ptr resource, uint mip_offset = 0);
			void createFrom2DArray(Resource::ptr resource, uint mip_offset = 0, uint array_offset = 0);

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

			void createFrom2D(Resource::ptr resource, uint mip_offset = 0);
			void createFrom2DArray(Resource::ptr resource, uint mip_offset = 0, uint array_offset = 0);

		};

	}
}

namespace HLSL
{
	void RaytracingAccelerationStructure::create(Resource::ptr resource)
	{
		HAL::Views::ShaderResource desc = { resource, HAL::Format::UNKNOWN, HAL::Views::ShaderResource::Raytracing {0} };
		Handle::operator=(desc);
	}

	void ByteAddressBuffer::create(Resource::ptr resource, uint64 offset, uint64 size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (size == 0) size = static_cast<uint>(buffer_desc.SizeInBytes / 4);

		HAL::Views::ShaderResource desc = { resource, Format::R32_TYPELESS, HAL::Views::ShaderResource::Buffer {(uint)offset, (uint)size, 0, true} };
		Handle::operator=(desc);
	}

	void RWByteAddressBuffer::create(Resource::ptr resource, uint64 offset, uint64 size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (size == 0) size = static_cast<uint64>(buffer_desc.SizeInBytes / 4);

		HAL::Views::UnorderedAccess desc = { resource, Format::R32_TYPELESS, HAL::Views::UnorderedAccess::Buffer {(uint)offset, (uint)size, 0, true, 0, nullptr} };
		Handle::operator=(desc);
	}



	template<class T>
	void StructuredBuffer<T>::create(Resource::ptr resource, uint64 first_elem, uint64 count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<uint>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::ShaderResource desc = { resource, Format::UNKNOWN, HAL::Views::ShaderResource::Buffer {first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false} };
		Handle::operator=(desc);
	}

	
	template<class T>
	void ConstantBuffer<T>::create(Resource::ptr resource, uint64 offset, uint64 size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
	
		HAL::Views::ConstantBuffer desc = { resource, offset, size};
		Handle::operator=(desc);
	}


	template<class T>
	void RWStructuredBuffer<T>::create(Resource::ptr resource, uint64 first_elem, uint64 count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<uint>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::UnorderedAccess desc = { resource, Format::UNKNOWN, HAL::Views::UnorderedAccess::Buffer {(uint)first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false, 0, nullptr} };
		Handle::operator=(desc);
	}


	template<class T>
	void AppendStructuredBuffer<T>::create(Resource::ptr counter_resource, uint64 counter_offset, Resource::ptr resource, uint64 first_elem, uint64 count)
	{
		HAL::Views::UnorderedAccess desc = { resource, Format::UNKNOWN, HAL::Views::UnorderedAccess::Buffer {(uint)first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false, counter_offset, counter_resource} };
		Handle::operator=(desc);
	}
	template<class T>
	void Buffer<T>::create(Resource::ptr resource, Format format, uint64 first_elem, uint64 count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<uint64>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::ShaderResource desc = { resource, format, HAL::Views::ShaderResource::Buffer {(uint)first_elem, static_cast<uint>(count), 0, false} };
		Handle::operator= (desc);
	}


	template<class T>
	void RWBuffer<T>::create(Resource::ptr resource, Format format, uint64 first_elem, uint64 count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<uint>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));


		HAL::Views::UnorderedAccess desc = { resource, format, HAL::Views::UnorderedAccess::Buffer {(uint)first_elem, static_cast<uint>(count), 0, false, 0, nullptr} };
		Handle::operator= (desc);
	}




	template<class T>
	void Texture2D<T>::create(Resource::ptr resource, uint first_mip, uint mip_levels, uint array_offset)
	{
		auto texture_desc = resource->get_desc().as_texture();

		bool is_array = texture_desc.ArraySize > 1;
		if (is_array)
		{
			auto desc = HAL::Views::ShaderResource{
				.Resource = resource,
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
				.Resource = resource,
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
	void RWTexture2D<T>::create(Resource::ptr resource, uint first_mip, uint array_offset)
	{
		auto texture_desc = resource->get_desc().as_texture();
		bool is_array = texture_desc.ArraySize > 1;


		if (!is_array)
		{
			auto desc = HAL::Views::UnorderedAccess{
			.Resource = resource,
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
				.Resource = resource,
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
	void Texture2DArray<T>::create(Resource::ptr resource, uint first_mip, uint mip_levels, uint array_offset, uint array_count)
	{
		auto desc = HAL::Views::ShaderResource{
			.Resource = resource,
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
	void RWTexture2DArray<T>::create(Resource::ptr resource, uint first_mip, uint mip_levels, uint array_offset, uint array_count)
	{
		auto desc = HAL::Views::UnorderedAccess{
				.Resource = resource,
				.Format = resource->get_desc().as_texture().Format.to_srv(),
				.View = HAL::Views::UnorderedAccess::Texture2DArray
					{
						.MipSlice = first_mip,
						.FirstArraySlice = array_offset,
						.ArraySize = array_count,
						.PlaneSlice = 0
					}
		};
		Handle::operator= (desc);
	}

	template<class T>
	void Texture3D<T>::create(Resource::ptr resource, uint first_mip, uint mip_levels)
	{
		auto desc = HAL::Views::ShaderResource{
				.Resource = resource,
				.Format = resource->get_desc().as_texture().Format.to_srv(),
				.View = HAL::Views::ShaderResource::Texture3D
						{
							.MostDetailedMip = first_mip,
							.MipLevels = mip_levels,
							.ResourceMinLODClamp = 0
						}
		};
		Handle::operator= (desc);
	}

	template<class T>
	void RWTexture3D<T>::create(Resource::ptr resource, uint first_mip)
	{
		auto desc = HAL::Views::UnorderedAccess{
				.Resource = resource,
				.Format = resource->get_desc().as_texture().Format.to_srv(),
				.View = HAL::Views::UnorderedAccess::Texture3D
						{
							.MipSlice = uint(first_mip),
							.FirstWSlice = 0,
							.WSize = resource->get_desc().as_texture().Dimensions.z >> uint(first_mip)
						}
		};
		Handle::operator= (desc);
	}

	template<class T>
	void TextureCube<T>::create(Resource::ptr resource, uint first_mip, uint mip_levels, uint cube_offset)
	{
		if (cube_offset > 0)
		{
			auto desc = HAL::Views::ShaderResource{
			resource,
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
				resource,
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
	void RenderTarget<T>::createFrom2D(Resource::ptr resource, uint first_mip)
	{
		auto desc = HAL::Views::RenderTarget{
			.Resource = resource,
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
	void RenderTarget<T>::createFrom2DArray(Resource::ptr resource, uint first_mip, uint array_id)
	{
		auto desc = HAL::Views::RenderTarget{
	.Resource = resource,
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
	void DepthStencil<T>::createFrom2D(Resource::ptr resource, uint first_mip)
	{
		auto desc = HAL::Views::DepthStencil{
		.Resource = resource,
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
	void DepthStencil<T>::createFrom2DArray(Resource::ptr resource, uint first_mip, uint array_id)
	{
		auto desc = HAL::Views::DepthStencil{
		.Resource = resource,
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