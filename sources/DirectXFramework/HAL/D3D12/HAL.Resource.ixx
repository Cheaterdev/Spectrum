export module HAL:Resource;
import d3d12;
import Math;
import Utils;

import :Types;
import :Sampler;

import :Utils;
import :Device;
import :Heap;

import stl.core;

import :Format;


HAL::ResourceDesc extract(D3D::Resource resource)
{
	auto native_desc = resource->GetDesc();

	if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		return HAL::ResourceDesc::Buffer(native_desc.Width, from_native(native_desc.Flags));
	}
	else
	{
		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
		{
			return  HAL::ResourceDesc::Tex1D(from_native(native_desc.Format), { uint(native_desc.Width) }, native_desc.DepthOrArraySize, native_desc.MipLevels, from_native(native_desc.Flags));
		}

		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{
			return  HAL::ResourceDesc::Tex2D(from_native(native_desc.Format), { uint(native_desc.Width), native_desc.Height }, native_desc.DepthOrArraySize, native_desc.MipLevels, from_native(native_desc.Flags));
		}

		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
		{
			return  HAL::ResourceDesc::Tex3D(from_native(native_desc.Format), { uint(native_desc.Width), native_desc.Height, native_desc.DepthOrArraySize }, native_desc.MipLevels, from_native(native_desc.Flags));
		}
	}

	assert(false);

	return HAL::ResourceDesc::Buffer(0);
}
export namespace HAL
{
	struct PlacementAddress
	{
		Heap* heap;
		size_t offset;
	};

	class Resource
	{
	protected:
		ResourceDesc desc;
	public:
		using ptr = std::shared_ptr<Resource>;
		void init(Device& device, const ResourceDesc& desc, const PlacementAddress& address, ResourceState initialState = ResourceState::GEN_READ);
		void init(D3D::Resource  resource)
		{
			native_resource = resource;
			desc = extract(resource);
		}
		std::span<std::byte> cpu_data();

		const ResourceDesc& get_desc() const
		{
			return desc;
		}

		GPUAddressPtr get_address()
		{
			assert(false);
			return 0;
		}
	public:
		D3D::Resource native_resource;

	private:
		SERIALIZE_PRETTY()
		{
			ar& NVP(desc);
		}
	};
}



export
{

	namespace cereal
	{
		template<class Archive>
		void serialize(Archive& ar, HAL::Resource*& g)
		{
			if (g)
			{
				auto desc = g->get_desc();
				auto native_desc = g->native_resource->GetDesc();

				ar& NVP(desc);
				ar& NVP(native_desc);
			}
		}
	}

}

namespace HAL
{
	void Resource::init(Device& device, const ResourceDesc& _desc, const PlacementAddress& address, ResourceState initialState)
	{
		desc = _desc;
		CD3DX12_RESOURCE_DESC resourceDesc = to_native(desc);


		if (desc.is_texture())
		{

			auto texture_desc = desc.as_texture();
			assert((texture_desc.is1D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE1D)
				|| (texture_desc.is2D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D)
				|| (texture_desc.is3D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE3D)
			);

			if (!texture_desc.Format.is_shader_visible())
			{
				desc.Flags = desc.Flags & (~HAL::ResFlags::ShaderResource);
			}

		}

		if (address.heap)
		{
			TEST(device,device.native_device->CreatePlacedResource(
				address.heap->native_heap.Get(),
				address.offset,
				&resourceDesc,
				static_cast<D3D12_RESOURCE_STATES>(initialState),
				nullptr,
				IID_PPV_ARGS(&native_resource)));
		}
		else
		{
			if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
				//resourceDesc.Alignment = 4 * 1024;
			}

			TEST(device,device.native_device->CreateReservedResource(
				&resourceDesc,
				static_cast<D3D12_RESOURCE_STATES>(initialState),
				nullptr,
				//	(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&native_resource)));
		}

		desc = extract(native_resource);

	}

}