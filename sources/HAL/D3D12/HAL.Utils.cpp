 module HAL:Utils;
 import :Resource;
import <HAL.h>;
import <stl/core.h>;

import <d3d12/d3d12_includes.h>;
import wrl;
import Core;

import :Types;
import :Sampler;
 

 
CD3DX12_RESOURCE_DESC  to_native_1(const ResourceDesc& desc)
{
	if (desc.is_buffer())
	{
		auto buffer_desc = desc.as_buffer();
		return CD3DX12_RESOURCE_DESC::Buffer(buffer_desc.SizeInBytes, to_native(desc.Flags));
	}
	else if (desc.is_texture())
	{
		auto texture_desc = desc.as_texture();


		if (texture_desc.is1D())
		{
			return  CD3DX12_RESOURCE_DESC::Tex1D(to_native(texture_desc.Format), texture_desc.Dimensions.x, texture_desc.ArraySize, texture_desc.MipLevels, to_native(desc.Flags));
		}

		if (texture_desc.is2D())
		{
			return  CD3DX12_RESOURCE_DESC::Tex2D(to_native(texture_desc.Format), texture_desc.Dimensions.x, texture_desc.Dimensions.y, texture_desc.ArraySize, texture_desc.MipLevels, 1, 0, to_native(desc.Flags));
		}

		if (texture_desc.is3D())
		{
			assert(texture_desc.ArraySize == 1);
			return  CD3DX12_RESOURCE_DESC::Tex3D(to_native(texture_desc.Format), texture_desc.Dimensions.x, texture_desc.Dimensions.y, texture_desc.Dimensions.z, texture_desc.MipLevels, to_native(desc.Flags));
		}
	}else if (desc.is_feedback())
	{
			assert(false);

	}
	assert(false);

	return CD3DX12_RESOURCE_DESC::Buffer(0, to_native(desc.Flags));
}


CD3DX12_RESOURCE_DESC1  to_native(const ResourceDesc& desc)
{
	if (desc.is_buffer())
	{
		auto buffer_desc = desc.as_buffer();
		return CD3DX12_RESOURCE_DESC1 ::Buffer(buffer_desc.SizeInBytes, to_native(desc.Flags));
	}
	else if (desc.is_texture())
	{
		auto texture_desc = desc.as_texture();


		if (texture_desc.is1D())
		{
			return  CD3DX12_RESOURCE_DESC1 ::Tex1D(to_native(texture_desc.Format), texture_desc.Dimensions.x, texture_desc.ArraySize, texture_desc.MipLevels, to_native(desc.Flags));
		}

		if (texture_desc.is2D())
		{
			return  CD3DX12_RESOURCE_DESC1 ::Tex2D(to_native(texture_desc.Format), texture_desc.Dimensions.x, texture_desc.Dimensions.y, texture_desc.ArraySize, texture_desc.MipLevels, 1, 0, to_native(desc.Flags));
		}

		if (texture_desc.is3D())
		{
			assert(texture_desc.ArraySize == 1);
			return  CD3DX12_RESOURCE_DESC1 ::Tex3D(to_native(texture_desc.Format), texture_desc.Dimensions.x, texture_desc.Dimensions.y, texture_desc.Dimensions.z, texture_desc.MipLevels, to_native(desc.Flags));
		}
	}else if (desc.is_feedback())
	{
			auto feedback_desc = desc.as_feedback();
			auto texture_desc = feedback_desc.texture->get_desc().as_texture();

		auto result = CD3DX12_RESOURCE_DESC1 ::Tex2D(DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE, texture_desc.Dimensions.x, texture_desc.Dimensions.y, texture_desc.ArraySize, texture_desc.MipLevels, 1, 0, to_native(desc.Flags));
		auto tiles= feedback_desc.texture->get_tiled_manager().get_tiles_count(0);
		

		auto res = uint2(4,4);

		while(res.x<tiles.x) res.x*=2;
		while(res.y<tiles.y) res.y*=2;

		result.SamplerFeedbackMipRegion = D3D12_MIP_REGION{res.x, res.y, 1 };


		return result;
	}
	assert(false);

	return CD3DX12_RESOURCE_DESC1 ::Buffer(0, to_native(desc.Flags));
}