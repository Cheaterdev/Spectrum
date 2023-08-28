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
		struct RaytracingAccelerationStructure : public SRVHandle
		{
			RaytracingAccelerationStructure() = default;
			RaytracingAccelerationStructure(const Handle& h) : SRVHandle(h)
			{

			}

			void create(const Resource::ptr& resource);
		};

		template<typename T> concept HasStructuredBuffer =
			requires (T t) {
			t.structuredBuffer;
		};


		template<typename T> concept HasRWStructuredBuffer =
			requires (T t) {
			t.rwStructuredBuffer;
		};


		template<typename T> concept HasAppendStructuredBuffer =
			requires (T t) {
			t.appendStructuredBuffer;
		};
		template<typename T> concept HasByteAddressBuffer =
			requires (T t) {
			t.byteAddressBuffer;
		};


		template<typename T> concept HasRWByteAddressBuffer =
			requires (T t) {
			t.rwByteAddressBuffer;
		};


		template<typename T> concept HasConstantBuffer =
			requires (T t) {
			t.constantBuffer;
		};

		template<class T>
		struct StructuredBuffer : public SRVHandle
		{
			using ptr = int;

			StructuredBuffer() = default;
			explicit  StructuredBuffer(const Handle& h) : SRVHandle(h)
			{

			}
			template<HasStructuredBuffer H>
			StructuredBuffer& operator= (const H& h)
			{
				this->operator=(h.structuredBuffer);

				return *this;
			}
			void create(const Resource::ptr& resource, uint64 first_elem = 0, uint64 count = 0);
		};

		template<class T>
		struct ConstantBuffer : public CBVHandle
		{
			using ptr = int;

			ConstantBuffer() = default;
			explicit  ConstantBuffer(const Handle& h) : CBVHandle(h)
			{

			}

			template<HasConstantBuffer H>
			auto operator= (const H& h)
			{
				this->operator=(h.constantBuffer);

				return *this;
			}

			void create(const Resource::ptr& resource, uint64 offset, uint size);
		};

		template<class T>
		struct RWStructuredBuffer : public UAVHandle
		{
			RWStructuredBuffer() = default;
			explicit  RWStructuredBuffer(const Handle& h) : UAVHandle(h)
			{

			}


			template<HasRWStructuredBuffer H>
			auto operator= (const H& h)
			{
				this->operator=(h.rwStructuredBuffer);

				return *this;
			}

			void create(const Resource::ptr& resource, uint64 first_elem = 0, uint64 count = 0);
		};

		template<class T>
		struct AppendStructuredBuffer : public UAVHandle
		{
			AppendStructuredBuffer() = default;
			explicit  AppendStructuredBuffer(const Handle& h) : UAVHandle(h)
			{

			}


			template<HasAppendStructuredBuffer H>
			auto operator= (const H& h)
			{
				this->operator=(h.appendStructuredBuffer);

				return *this;
			}

			void create(const Resource::ptr& counter_resource, uint64 counter_offset, const Resource::ptr& resource, uint64 first_elem = 0, uint64 count = 0);
		};

		struct ByteAddressBuffer : public SRVHandle
		{
			ByteAddressBuffer() = default;
			explicit ByteAddressBuffer(const Handle& h) : SRVHandle(h)
			{

			}


			template<HasByteAddressBuffer H>
			auto operator= (const H& h)
			{
				this->operator=(h.byteAddressBuffer);

				return *this;
			}
			void create(const Resource::ptr& resource, uint64 offset = 0, uint64 size = 0);
		};


		struct RWByteAddressBuffer : public UAVHandle
		{
			RWByteAddressBuffer() = default;
			explicit RWByteAddressBuffer(const Handle& h) : UAVHandle(h)
			{

			}

			template<HasRWByteAddressBuffer H>
			auto operator= (const H& h)
			{
				this->operator=(h.rwByteAddressBuffer);

				return *this;
			}
			void create(const Resource::ptr& resource, uint64 offset = 0, uint64 size = 0);
		};

		template<class T>
		struct Buffer : public SRVHandle
		{
			Buffer() = default;
			explicit  Buffer(const Handle& h) : SRVHandle(h)
			{

			}

			void create(const Resource::ptr& resource, Format format, uint64 offset = 0, uint64 size = 0);
		};

		template<class T>
		struct RWBuffer : public UAVHandle
		{
			RWBuffer() = default;
			explicit RWBuffer(const Handle& h) : UAVHandle(h)
			{

			}
			void create(const Resource::ptr& resource, Format format, uint64 offset = 0, uint64 size = 0);
		};



		template<typename T> concept HasTexture2D =
			requires (T t) {
			t.texture2D;
		};


		template<class T = float4>
		struct Texture2D : public SRVHandle
		{
			Texture2D() = default;
			explicit Texture2D(const Handle& h) : SRVHandle(h)
			{

			}

			template<class T2>
			Texture2D(const Texture2D<T2>& h) : SRVHandle(h)
			{

			}

			template<HasTexture2D H>
			auto operator= (const H& h)
			{
				this->operator=(h.texture2D);

				return *this;
			}


			void create(const Resource::ptr& resource, uint first_mip = 1, uint mip_levels = 0, uint array_offset = 0);
		};


		template<typename T> concept HasTexture2DArray =
			requires (T t) {
			t.texture2DArray;
		};


		template<class T = float4>
		struct Texture2DArray : public SRVHandle
		{

			Texture2DArray() = default;
			explicit Texture2DArray(const Handle& h) : SRVHandle(h)
			{

			}

			template<class T2>
			Texture2DArray(const Texture2DArray<T2>& h) : SRVHandle(h)
			{

			}


			template<HasTexture2DArray H>
			auto operator= (const H& h)
			{
				this->operator=(h.texture2DArray);

				return *this;
			}


			void create(const Resource::ptr& resource, uint first_mip = 1, uint mip_levels = 0, uint array_offset = 0, uint array_count = 1);
		};


		template<typename T> concept HasTexture3D =
			requires (T t) {
			t.texture3D;
		};


		template<class T = float4>
		struct Texture3D : public SRVHandle
		{
			Texture3D() = default;
			explicit Texture3D(const Handle& h) : SRVHandle(h)
			{

			}

			template<class T2>
			Texture3D(const Texture3D<T2>& h) : SRVHandle(h)
			{

			}

			template<HasTexture3D H>
			auto operator= (const H& h)
			{
				this->operator=(h.texture3D);

				return *this;
			}


			void create(const Resource::ptr& resource, uint first_mip = 1, uint mip_levels = 0);
		};


		template<typename T> concept HasTextureCube =
			requires (T t) {
			t.textureCube;
		};


		template<class T = float4>
		struct TextureCube : public SRVHandle
		{

			TextureCube() = default;
			explicit TextureCube(const Handle& h) : SRVHandle(h)
			{

			}

			template<class T2>
			TextureCube(const TextureCube<T2>& h) : SRVHandle(h)
			{

			}


			template<HasTextureCube H>
			auto operator= (const H& h)
			{
				this->operator=(h.textureCube);

				return *this;
			}


			void create(const Resource::ptr& resource, uint first_mip = 1, uint mip_levels = 0, uint array_offset = 0);
		};
		template<typename T> concept HasRWTexture2D =
			requires (T t) {
			t.rwTexture2D;
		};

		template<class T = float4>
		struct RWTexture2D : public UAVHandle
		{
			RWTexture2D() = default;
			explicit RWTexture2D(const Handle& h) : UAVHandle(h)
			{

			}

			template<class T2>
			RWTexture2D(const RWTexture2D<T2>& h) : UAVHandle(h)
			{

			}


			template<HasRWTexture2D H>
			auto operator= (const H& h)
			{
				this->operator=(h.rwTexture2D);

				return *this;
			}


			void create(const Resource::ptr& resource, uint first_mip = 1, uint array_offset = 0);
		};
		template<typename T> concept HasRWTexture2DArray =
			requires (T t) {
			t.rwTexture2DArray;
		};
		template<class T = float4>
		struct RWTexture2DArray : public UAVHandle
		{
			RWTexture2DArray() = default;
			explicit RWTexture2DArray(const Handle& h) : UAVHandle(h)
			{

			}

			template<class T2>
			RWTexture2DArray(const RWTexture2DArray<T2>& h) : Handle(h)
			{

			}


			template<HasRWTexture2DArray H>
			auto operator= (const H& h)
			{
				this->operator=(h.rwTexture2DArray);

				return *this;
			}
			void create(const Resource::ptr& resource, uint first_mip = 0, uint mip_levels = 1, uint array_offset = 0, uint array_count = 1);
		};

		template<typename T> concept HasRWTexture3D =
			requires (T t) {
			t.rwTexture3D;
		};
		template<class T = float4>
		struct RWTexture3D : public UAVHandle
		{
			RWTexture3D() = default;
			explicit RWTexture3D(const Handle& h) : UAVHandle(h)
			{

			}
			template<class T2>
			RWTexture3D(const RWTexture3D<T2>& h) : UAVHandle(h)
			{

			}
			template<HasRWTexture3D H>
			auto operator= (const H& h)
			{
				this->operator=(h.rwTexture3D);

				return *this;
			}
			void create(const Resource::ptr& resource, uint first_mip = 1);

		};

		template<class T = float4>
		struct RenderTarget : public RTVHandle
		{

			RenderTarget() = default;
			explicit RenderTarget(const Handle& h) : RTVHandle(h)
			{

			}

			template<class T2>
			RenderTarget(const RenderTarget<T2>& h) : RTVHandle(h)
			{

			}

			void createFrom2D(const Resource::ptr& resource, uint mip_offset = 0);
			void createFrom2DArray(const Resource::ptr& resource, uint mip_offset = 0, uint array_offset = 0);

		};

		template<class T = float>
		struct DepthStencil : public DSVHandle
		{
			DepthStencil() = default;
			explicit DepthStencil(const Handle& h) : DSVHandle(h)
			{

			}

			template<class T2>
			DepthStencil(const RenderTarget<T2>& h) : DSVHandle(h)
			{

			}

			void createFrom2D(const Resource::ptr& resource, uint mip_offset = 0);
			void createFrom2DArray(const Resource::ptr& resource, uint mip_offset = 0, uint array_offset = 0);

		};

	}
}

namespace HLSL
{
	void RaytracingAccelerationStructure::create(const Resource::ptr& resource)
	{
		HAL::Views::ShaderResource desc = { resource, HAL::Format::UNKNOWN, HAL::Views::ShaderResource::Raytracing {0} };
		Handle::operator=(desc);
	}

	void ByteAddressBuffer::create(const Resource::ptr& resource, uint64 offset, uint64 size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (size == 0) size = static_cast<uint>(buffer_desc.SizeInBytes / 4);

		HAL::Views::ShaderResource desc = { resource, Format::R32_TYPELESS, HAL::Views::ShaderResource::Buffer {(uint)offset, (uint)size, 0, true} };
		Handle::operator=(desc);
	}

	void RWByteAddressBuffer::create(const Resource::ptr& resource, uint64 offset, uint64 size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (size == 0) size = static_cast<uint64>(buffer_desc.SizeInBytes / 4);

		HAL::Views::UnorderedAccess desc = { resource, Format::R32_TYPELESS, HAL::Views::UnorderedAccess::Buffer {(uint)offset, (uint)size, 0, true, 0, nullptr} };
		Handle::operator=(desc);
	}



	template<class T>
	void StructuredBuffer<T>::create(const Resource::ptr& resource, uint64 first_elem, uint64 count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<uint>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::ShaderResource desc = { resource, Format::UNKNOWN, HAL::Views::ShaderResource::Buffer {first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false} };
		Handle::operator=(desc);
	}


	template<class T>
	void ConstantBuffer<T>::create(const Resource::ptr& resource, uint64 offset, uint size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();

		HAL::Views::ConstantBuffer desc = { resource, offset, size };
		Handle::operator=(desc);
	}


	template<class T>
	void RWStructuredBuffer<T>::create(const Resource::ptr& resource, uint64 first_elem, uint64 count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<uint>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::UnorderedAccess desc = { resource, Format::UNKNOWN, HAL::Views::UnorderedAccess::Buffer {(uint)first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false, 0, nullptr} };
		Handle::operator=(desc);
	}


	template<class T>
	void AppendStructuredBuffer<T>::create(const Resource::ptr& counter_resource, uint64 counter_offset, const Resource::ptr& resource, uint64 first_elem, uint64 count)
	{
		HAL::Views::UnorderedAccess desc = { resource, Format::UNKNOWN, HAL::Views::UnorderedAccess::Buffer {(uint)first_elem, static_cast<uint>(count), sizeof(Underlying<T>), false, counter_offset, counter_resource} };
		Handle::operator=(desc);
	}
	template<class T>
	void Buffer<T>::create(const Resource::ptr& resource, Format format, uint64 first_elem, uint64 count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<uint64>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));

		HAL::Views::ShaderResource desc = { resource, format, HAL::Views::ShaderResource::Buffer {(uint)first_elem, static_cast<uint>(count), 0, false} };
		Handle::operator= (desc);
	}


	template<class T>
	void RWBuffer<T>::create(const Resource::ptr& resource, Format format, uint64 first_elem, uint64 count)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (count == 0) count = static_cast<uint>(buffer_desc.SizeInBytes / sizeof(Underlying<T>));


		HAL::Views::UnorderedAccess desc = { resource, format, HAL::Views::UnorderedAccess::Buffer {(uint)first_elem, static_cast<uint>(count), 0, false, 0, nullptr} };
		Handle::operator= (desc);
	}




	template<class T>
	void Texture2D<T>::create(const Resource::ptr& resource, uint first_mip, uint mip_levels, uint array_offset)
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
	void RWTexture2D<T>::create(const Resource::ptr& resource, uint first_mip, uint array_offset)
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
	void Texture2DArray<T>::create(const Resource::ptr& resource, uint first_mip, uint mip_levels, uint array_offset, uint array_count)
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
	void RWTexture2DArray<T>::create(const Resource::ptr& resource, uint first_mip, uint mip_levels, uint array_offset, uint array_count)
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
	void Texture3D<T>::create(const Resource::ptr& resource, uint first_mip, uint mip_levels)
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
	void RWTexture3D<T>::create(const Resource::ptr& resource, uint first_mip)
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
	void TextureCube<T>::create(const Resource::ptr& resource, uint first_mip, uint mip_levels, uint cube_offset)
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
	void RenderTarget<T>::createFrom2D(const Resource::ptr& resource, uint first_mip)
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
	void RenderTarget<T>::createFrom2DArray(const Resource::ptr& resource, uint first_mip, uint array_id)
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
	void DepthStencil<T>::createFrom2D(const Resource::ptr& resource, uint first_mip)
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
	void DepthStencil<T>::createFrom2DArray(const Resource::ptr& resource, uint first_mip, uint array_id)
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