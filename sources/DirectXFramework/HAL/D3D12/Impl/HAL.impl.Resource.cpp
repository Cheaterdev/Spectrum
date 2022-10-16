module Graphics:Resource;

import HAL;
import d3d12;
import Math;
import Utils;

#undef THIS


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


namespace HAL
{
	void Resource::init(Device& device, const ResourceDesc& _desc, const PlacementAddress& address, ResourceState initialState)
	{
		auto THIS = static_cast<Graphics::Resource*>(this);


		THIS->desc = _desc;
		CD3DX12_RESOURCE_DESC resourceDesc = to_native(THIS->desc);


		if (THIS->desc.is_texture())
		{

			auto texture_desc = THIS->desc.as_texture();
			assert((texture_desc.is1D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE1D)
				|| (texture_desc.is2D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D)
				|| (texture_desc.is3D() && resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE3D)
			);

			if (!texture_desc.Format.is_shader_visible())
			{
				THIS->desc.Flags &= (~HAL::ResFlags::ShaderResource);
			}

		}

		if (address.heap)
		{
			TEST(device, device.native_device->CreatePlacedResource(
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

			TEST(device, device.native_device->CreateReservedResource(
				&resourceDesc,
				static_cast<D3D12_RESOURCE_STATES>(initialState),
				nullptr,
				//	(desc.Dimension == D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_TEXTURE2D && (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))) ? &value : nullptr,
				IID_PPV_ARGS(&native_resource)));
		}

		THIS->desc = extract(native_resource);
		this->address = GPUAddressPtr(native_resource->GetGPUVirtualAddress());

	}
	void Resource::init(D3D::Resource  resource)
	{
		auto THIS = static_cast<Graphics::Resource*>(this);

		native_resource = resource;
		THIS->desc = extract(resource);
	}
}
namespace Graphics
{
	
}