export module D3D12.Utils;
import d3d12;
import Vectors;
import HAL.Types;
import HAL.Sampler;
import HAL.Format;

using namespace HAL;

export namespace D3D
{
	using Heap = ComPtr<ID3D12Heap>;
	using Resource = ComPtr<ID3D12Resource>;
	using Device = ComPtr<ID3D12Device5>;
	using Adapter = ComPtr<IDXGIAdapter3>;

}

export D3D12_TEXTURE_ADDRESS_MODE to_native(TextureAddressMode mode)
{
	static constexpr D3D12_TEXTURE_ADDRESS_MODE natives[] =
	{
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE
	};

	return natives[static_cast<uint>(mode)];
}

export D3D12_COMPARISON_FUNC to_native(ComparisonFunc mode)
{
	static constexpr D3D12_COMPARISON_FUNC natives[] =
	{
		D3D12_COMPARISON_FUNC_NEVER,
		D3D12_COMPARISON_FUNC_LESS,
		D3D12_COMPARISON_FUNC_EQUAL,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_COMPARISON_FUNC_GREATER,
		D3D12_COMPARISON_FUNC_NOT_EQUAL,
		D3D12_COMPARISON_FUNC_GREATER_EQUAL,
		D3D12_COMPARISON_FUNC_ALWAYS,

		D3D12_COMPARISON_FUNC_NEVER,
		D3D12_COMPARISON_FUNC_NEVER,
		D3D12_COMPARISON_FUNC_NEVER,
	};

	return natives[static_cast<uint>(mode)];
}


export D3D12_FILTER to_native(Filter min, Filter mag, Filter mip, ComparisonFunc func)
{
	static constexpr D3D12_FILTER natives[] =
	{
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,
		D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT,
		D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT,
		D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
		D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
		D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_FILTER_COMPARISON_ANISOTROPIC,
		D3D12_FILTER_MINIMUM_MIN_MAG_MIP_POINT,
		D3D12_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
		D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
		D3D12_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR,
		D3D12_FILTER_MINIMUM_ANISOTROPIC,
		D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_POINT,
		D3D12_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
		D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
		D3D12_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		D3D12_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR,
		D3D12_FILTER_MAXIMUM_ANISOTROPIC
	};

	uint index = static_cast<uint>(mip) + 4 * static_cast<uint>(min) + 2 * static_cast<uint>(mag);
	if (min == Filter::ANISOTROPIC)
		index = 8;

	if (func == ComparisonFunc::MIN)
	{
		index += 17;
	}
	else if (func == ComparisonFunc::MAX)
	{
		index += 25;
	}
	else if (func != ComparisonFunc::NONE)
	{
		index += 9;
	}

	return natives[index];
}



export D3D12_SAMPLER_DESC to_native(const SamplerDesc& desc)
{
	D3D12_SAMPLER_DESC result;

	result.Filter = to_native(desc.MinFilter, desc.MagFilter, desc.MipFilter, desc.ComparisonFunc);

	result.ComparisonFunc = to_native(desc.ComparisonFunc);
	result.AddressU = to_native(desc.AddressU);
	result.AddressW = to_native(desc.AddressW);
	result.AddressV = to_native(desc.AddressV);

	result.BorderColor[0] = desc.BorderColor.x;
	result.BorderColor[1] = desc.BorderColor.y;
	result.BorderColor[2] = desc.BorderColor.z;
	result.BorderColor[3] = desc.BorderColor.w;

	result.MaxAnisotropy = desc.MaxAnisotropy;
	result.MipLODBias = desc.MipLODBias;
	result.MinLOD = desc.MinLOD;
	result.MaxLOD = desc.MaxLOD;

	return result;
}


export D3D12_COMMAND_LIST_TYPE to_native(CommandListType type)
{
	static constexpr D3D12_COMMAND_LIST_TYPE natives[] =
	{
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_LIST_TYPE_COMPUTE,
		D3D12_COMMAND_LIST_TYPE_COPY
	};

	return natives[static_cast<uint>(type)];
}



export D3D12_HEAP_TYPE to_native(HeapType type)
{
	static constexpr D3D12_HEAP_TYPE natives[] =
	{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_HEAP_TYPE_UPLOAD,
		D3D12_HEAP_TYPE_READBACK
	};

	return natives[static_cast<uint>(type)];
}



export D3D12_HEAP_FLAGS to_native(HeapFlags flags)
{
	static constexpr D3D12_HEAP_FLAGS  natives[] =
	{
		D3D12_HEAP_FLAG_NONE ,
		D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS,
		D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES,
		D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES
	};

	return natives[static_cast<uint>(flags)];
}



export D3D12_DESCRIPTOR_HEAP_FLAGS to_native(DescriptorHeapFlags flags)
{
	static constexpr D3D12_DESCRIPTOR_HEAP_FLAGS natives[] =
	{
		D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};

	return natives[static_cast<uint>(flags)];
}



export D3D12_DESCRIPTOR_HEAP_TYPE to_native(DescriptorHeapType type)
{
	static constexpr D3D12_DESCRIPTOR_HEAP_TYPE natives[] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	};

	return natives[static_cast<uint>(type)];
}



export D3D12_CULL_MODE to_native(CullMode mode)
{
	static constexpr D3D12_CULL_MODE natives[] =
	{
		D3D12_CULL_MODE::D3D12_CULL_MODE_NONE,
		D3D12_CULL_MODE::D3D12_CULL_MODE_BACK,
		D3D12_CULL_MODE::D3D12_CULL_MODE_FRONT
	};

	return natives[static_cast<uint>(mode)];
}


export D3D12_FILL_MODE to_native(FillMode mode)
{
	static constexpr D3D12_FILL_MODE natives[] =
	{
		D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME,
		D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID
	};

	return natives[static_cast<uint>(mode)];
}



export D3D12_BLEND to_native(Blend blend)
{
	static constexpr D3D12_BLEND natives[] =
	{
		D3D12_BLEND::D3D12_BLEND_ZERO,
		D3D12_BLEND::D3D12_BLEND_ONE,
		D3D12_BLEND::D3D12_BLEND_SRC_COLOR,
		D3D12_BLEND::D3D12_BLEND_INV_SRC_COLOR,
		D3D12_BLEND::D3D12_BLEND_SRC_ALPHA,
		D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA,
		D3D12_BLEND::D3D12_BLEND_DEST_ALPHA,
		D3D12_BLEND::D3D12_BLEND_INV_DEST_ALPHA,
		D3D12_BLEND::D3D12_BLEND_DEST_COLOR,
		D3D12_BLEND::D3D12_BLEND_INV_DEST_COLOR,
		D3D12_BLEND::D3D12_BLEND_SRC_ALPHA_SAT,
		D3D12_BLEND::D3D12_BLEND_BLEND_FACTOR,
		D3D12_BLEND::D3D12_BLEND_INV_BLEND_FACTOR,
		D3D12_BLEND::D3D12_BLEND_SRC1_COLOR,
		D3D12_BLEND::D3D12_BLEND_INV_SRC1_COLOR,
		D3D12_BLEND::D3D12_BLEND_SRC1_ALPHA,
		D3D12_BLEND::D3D12_BLEND_INV_SRC1_ALPHA
	};

	return natives[static_cast<uint>(blend)];
}


export D3D12_STENCIL_OP to_native(StencilOp mode)
{
	static constexpr D3D12_STENCIL_OP natives[] =
	{
		D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP::D3D12_STENCIL_OP_ZERO,
		D3D12_STENCIL_OP::D3D12_STENCIL_OP_REPLACE,
		D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR_SAT,
		D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR_SAT,
		D3D12_STENCIL_OP::D3D12_STENCIL_OP_INVERT,
		D3D12_STENCIL_OP::D3D12_STENCIL_OP_INCR,
		D3D12_STENCIL_OP::D3D12_STENCIL_OP_DECR,
	};

	return natives[static_cast<uint>(mode)];
}


export D3D12_DEPTH_STENCILOP_DESC to_native(StencilDesc desc)
{
	D3D12_DEPTH_STENCILOP_DESC result;
	result.StencilFailOp = to_native(desc.StencilFailOp);
	result.StencilDepthFailOp = to_native(desc.StencilDepthFailOp);
	result.StencilPassOp = to_native(desc.StencilPassOp);
	result.StencilFunc = to_native(desc.StencilFunc);

	return result;
}


export DXGI_FORMAT to_native(Format format)
{
	return static_cast<DXGI_FORMAT>(format.get_native());
}

bool Format::is_shader_visible() const
{
	switch (native_format)
	{

	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return false;
	default:
		return true;
	}
}


Format Format::to_typeless() const
{

	switch (native_format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return HAL::Formats::R8G8B8A8_TYPELESS;

	default:
		return *this;
	}
}

namespace HAL
{
	const Format Formats::Unknown = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

	 const Format Formats::R8G8B8A8_UNORM_SRGB = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	 const Format Formats::R8G8B8A8_UNORM = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	 const Format Formats::R8G8B8A8_UINT = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UINT;
	 const Format Formats::R8G8B8A8_SNORM = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_SNORM;
	 const Format Formats::R8G8B8A8_SINT = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_SINT;

	 const Format Formats::R8G8B8A8_TYPELESS = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_TYPELESS;
}