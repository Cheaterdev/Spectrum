export module HAL:Utils;
export import "macros.h";


import d3d12;
import Math;
import Log;

import :Types;
import :Sampler;
using namespace HAL;

export namespace D3D
{
	using Heap = ComPtr<ID3D12Heap>;
	using Resource = ComPtr<ID3D12Resource>;
	using Device = ComPtr<ID3D12Device5>;
	using Fence = ComPtr<ID3D12Fence>;
	using CommandList = ComPtr<ID3D12GraphicsCommandList4>;
	using CommandAllocator = ComPtr<ID3D12CommandAllocator>;
	using CommandSignature = ComPtr<ID3D12CommandSignature>;
	using PipelineState = ComPtr<ID3D12PipelineState>;
	using StateObject = ComPtr<ID3D12StateObject>;
	using StateObjectProperties = ComPtr<ID3D12StateObjectProperties>;
	using Blob = ComPtr<ID3DBlob>;
	using DescriptorHeap = ComPtr<ID3D12DescriptorHeap>;
	using QueryHeap = ComPtr<ID3D12QueryHeap>;
	using RootSignature = ComPtr<ID3D12RootSignature>;
	using CommandQueue = ComPtr<ID3D12CommandQueue>;

	using Debug = ComPtr<ID3D12Debug1>;
	using InfoQueue = ComPtr<ID3D12InfoQueue>;
}

export namespace DXGI
{
	using Adapter = ComPtr<IDXGIAdapter3>;
	using Swapchain = ComPtr<IDXGISwapChain3>;
	using Surface = ComPtr<IDXGISurface2>;
	using Device = ComPtr<IDXGIDevice2>;
	using Factory = ComPtr<IDXGIFactory2>;
	using Output = ComPtr<IDXGIOutput1>;
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
	static const DXGI_FORMAT ar[] = {
	DXGI_FORMAT_UNKNOWN,
	DXGI_FORMAT_R32G32B32A32_TYPELESS,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_R32G32B32A32_UINT,
	DXGI_FORMAT_R32G32B32A32_SINT,
	DXGI_FORMAT_R32G32B32_TYPELESS,
	DXGI_FORMAT_R32G32B32_FLOAT,
	DXGI_FORMAT_R32G32B32_UINT,
	DXGI_FORMAT_R32G32B32_SINT,
	DXGI_FORMAT_R16G16B16A16_TYPELESS,
	DXGI_FORMAT_R16G16B16A16_FLOAT,
	DXGI_FORMAT_R16G16B16A16_UNORM,
	DXGI_FORMAT_R16G16B16A16_UINT,
	DXGI_FORMAT_R16G16B16A16_SNORM,
	DXGI_FORMAT_R16G16B16A16_SINT,
	DXGI_FORMAT_R32G32_TYPELESS,
	DXGI_FORMAT_R32G32_FLOAT,
	DXGI_FORMAT_R32G32_UINT,
	DXGI_FORMAT_R32G32_SINT,
	DXGI_FORMAT_R32G8X24_TYPELESS,
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
	DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
	DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
	DXGI_FORMAT_R10G10B10A2_TYPELESS,
	DXGI_FORMAT_R10G10B10A2_UNORM,
	DXGI_FORMAT_R10G10B10A2_UINT,
	DXGI_FORMAT_R11G11B10_FLOAT,
	DXGI_FORMAT_R8G8B8A8_TYPELESS,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
	DXGI_FORMAT_R8G8B8A8_UINT,
	DXGI_FORMAT_R8G8B8A8_SNORM,
	DXGI_FORMAT_R8G8B8A8_SINT,
	DXGI_FORMAT_R16G16_TYPELESS,
	DXGI_FORMAT_R16G16_FLOAT,
	DXGI_FORMAT_R16G16_UNORM,
	DXGI_FORMAT_R16G16_UINT,
	DXGI_FORMAT_R16G16_SNORM,
	DXGI_FORMAT_R16G16_SINT,
	DXGI_FORMAT_R32_TYPELESS,
	DXGI_FORMAT_D32_FLOAT,
	DXGI_FORMAT_R32_FLOAT,
	DXGI_FORMAT_R32_UINT,
	DXGI_FORMAT_R32_SINT,
	DXGI_FORMAT_R24G8_TYPELESS,
	DXGI_FORMAT_D24_UNORM_S8_UINT,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
	DXGI_FORMAT_X24_TYPELESS_G8_UINT,
	DXGI_FORMAT_R8G8_TYPELESS,
	DXGI_FORMAT_R8G8_UNORM,
	DXGI_FORMAT_R8G8_UINT,
	DXGI_FORMAT_R8G8_SNORM,
	DXGI_FORMAT_R8G8_SINT,
	DXGI_FORMAT_R16_TYPELESS,
	DXGI_FORMAT_R16_FLOAT,
	DXGI_FORMAT_D16_UNORM,
	DXGI_FORMAT_R16_UNORM,
	DXGI_FORMAT_R16_UINT,
	DXGI_FORMAT_R16_SNORM,
	DXGI_FORMAT_R16_SINT,
	DXGI_FORMAT_R8_TYPELESS,
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R8_UINT,
	DXGI_FORMAT_R8_SNORM,
	DXGI_FORMAT_R8_SINT,
	DXGI_FORMAT_A8_UNORM,
	DXGI_FORMAT_R1_UNORM,
	DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
	DXGI_FORMAT_R8G8_B8G8_UNORM,
	DXGI_FORMAT_G8R8_G8B8_UNORM,
	DXGI_FORMAT_BC1_TYPELESS,
	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC1_UNORM_SRGB,
	DXGI_FORMAT_BC2_TYPELESS,
	DXGI_FORMAT_BC2_UNORM,
	DXGI_FORMAT_BC2_UNORM_SRGB,
	DXGI_FORMAT_BC3_TYPELESS,
	DXGI_FORMAT_BC3_UNORM,
	DXGI_FORMAT_BC3_UNORM_SRGB,
	DXGI_FORMAT_BC4_TYPELESS,
	DXGI_FORMAT_BC4_UNORM,
	DXGI_FORMAT_BC4_SNORM,
	DXGI_FORMAT_BC5_TYPELESS,
	DXGI_FORMAT_BC5_UNORM,
	DXGI_FORMAT_BC5_SNORM,
	DXGI_FORMAT_B5G6R5_UNORM,
	DXGI_FORMAT_B5G5R5A1_UNORM,
	DXGI_FORMAT_B8G8R8A8_UNORM,
	DXGI_FORMAT_B8G8R8X8_UNORM,
	DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
	DXGI_FORMAT_B8G8R8A8_TYPELESS,
	DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
	DXGI_FORMAT_B8G8R8X8_TYPELESS,
	DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
	DXGI_FORMAT_BC6H_TYPELESS,
	DXGI_FORMAT_BC6H_UF16,
	DXGI_FORMAT_BC6H_SF16,
	DXGI_FORMAT_BC7_TYPELESS,
	DXGI_FORMAT_BC7_UNORM,
	DXGI_FORMAT_BC7_UNORM_SRGB };

	return ar[format.get_native()];
}


export Format from_native(DXGI_FORMAT format)
{
	if (format == DXGI_FORMAT_UNKNOWN)  return Format::UNKNOWN;
	if (format == DXGI_FORMAT_R32G32B32A32_TYPELESS)  return Format::R32G32B32A32_TYPELESS;
	if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)  return Format::R32G32B32A32_FLOAT;
	if (format == DXGI_FORMAT_R32G32B32A32_UINT)  return Format::R32G32B32A32_UINT;
	if (format == DXGI_FORMAT_R32G32B32A32_SINT)  return Format::R32G32B32A32_SINT;
	if (format == DXGI_FORMAT_R32G32B32_TYPELESS)  return Format::R32G32B32_TYPELESS;
	if (format == DXGI_FORMAT_R32G32B32_FLOAT)  return Format::R32G32B32_FLOAT;
	if (format == DXGI_FORMAT_R32G32B32_UINT)  return Format::R32G32B32_UINT;
	if (format == DXGI_FORMAT_R32G32B32_SINT)  return Format::R32G32B32_SINT;
	if (format == DXGI_FORMAT_R16G16B16A16_TYPELESS)  return Format::R16G16B16A16_TYPELESS;
	if (format == DXGI_FORMAT_R16G16B16A16_FLOAT)  return Format::R16G16B16A16_FLOAT;
	if (format == DXGI_FORMAT_R16G16B16A16_UNORM)  return Format::R16G16B16A16_UNORM;
	if (format == DXGI_FORMAT_R16G16B16A16_UINT)  return Format::R16G16B16A16_UINT;
	if (format == DXGI_FORMAT_R16G16B16A16_SNORM)  return Format::R16G16B16A16_SNORM;
	if (format == DXGI_FORMAT_R16G16B16A16_SINT)  return Format::R16G16B16A16_SINT;
	if (format == DXGI_FORMAT_R32G32_TYPELESS)  return Format::R32G32_TYPELESS;
	if (format == DXGI_FORMAT_R32G32_FLOAT)  return Format::R32G32_FLOAT;
	if (format == DXGI_FORMAT_R32G32_UINT)  return Format::R32G32_UINT;
	if (format == DXGI_FORMAT_R32G32_SINT)  return Format::R32G32_SINT;
	if (format == DXGI_FORMAT_R32G8X24_TYPELESS)  return Format::R32G8X24_TYPELESS;
	if (format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)  return Format::D32_FLOAT_S8X24_UINT;
	if (format == DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS)  return Format::R32_FLOAT_X8X24_TYPELESS;
	if (format == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT)  return Format::X32_TYPELESS_G8X24_UINT;
	if (format == DXGI_FORMAT_R10G10B10A2_TYPELESS)  return Format::R10G10B10A2_TYPELESS;
	if (format == DXGI_FORMAT_R10G10B10A2_UNORM)  return Format::R10G10B10A2_UNORM;
	if (format == DXGI_FORMAT_R10G10B10A2_UINT)  return Format::R10G10B10A2_UINT;
	if (format == DXGI_FORMAT_R11G11B10_FLOAT)  return Format::R11G11B10_FLOAT;
	if (format == DXGI_FORMAT_R8G8B8A8_TYPELESS)  return Format::R8G8B8A8_TYPELESS;
	if (format == DXGI_FORMAT_R8G8B8A8_UNORM)  return Format::R8G8B8A8_UNORM;
	if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)  return Format::R8G8B8A8_UNORM_SRGB;
	if (format == DXGI_FORMAT_R8G8B8A8_UINT)  return Format::R8G8B8A8_UINT;
	if (format == DXGI_FORMAT_R8G8B8A8_SNORM)  return Format::R8G8B8A8_SNORM;
	if (format == DXGI_FORMAT_R8G8B8A8_SINT)  return Format::R8G8B8A8_SINT;
	if (format == DXGI_FORMAT_R16G16_TYPELESS)  return Format::R16G16_TYPELESS;
	if (format == DXGI_FORMAT_R16G16_FLOAT)  return Format::R16G16_FLOAT;
	if (format == DXGI_FORMAT_R16G16_UNORM)  return Format::R16G16_UNORM;
	if (format == DXGI_FORMAT_R16G16_UINT)  return Format::R16G16_UINT;
	if (format == DXGI_FORMAT_R16G16_SNORM)  return Format::R16G16_SNORM;
	if (format == DXGI_FORMAT_R16G16_SINT)  return Format::R16G16_SINT;
	if (format == DXGI_FORMAT_R32_TYPELESS)  return Format::R32_TYPELESS;
	if (format == DXGI_FORMAT_D32_FLOAT)  return Format::D32_FLOAT;
	if (format == DXGI_FORMAT_R32_FLOAT)  return Format::R32_FLOAT;
	if (format == DXGI_FORMAT_R32_UINT)  return Format::R32_UINT;
	if (format == DXGI_FORMAT_R32_SINT)  return Format::R32_SINT;
	if (format == DXGI_FORMAT_R24G8_TYPELESS)  return Format::R24G8_TYPELESS;
	if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)  return Format::D24_UNORM_S8_UINT;
	if (format == DXGI_FORMAT_R24_UNORM_X8_TYPELESS)  return Format::R24_UNORM_X8_TYPELESS;
	if (format == DXGI_FORMAT_X24_TYPELESS_G8_UINT)  return Format::X24_TYPELESS_G8_UINT;
	if (format == DXGI_FORMAT_R8G8_TYPELESS)  return Format::R8G8_TYPELESS;
	if (format == DXGI_FORMAT_R8G8_UNORM)  return Format::R8G8_UNORM;
	if (format == DXGI_FORMAT_R8G8_UINT)  return Format::R8G8_UINT;
	if (format == DXGI_FORMAT_R8G8_SNORM)  return Format::R8G8_SNORM;
	if (format == DXGI_FORMAT_R8G8_SINT)  return Format::R8G8_SINT;
	if (format == DXGI_FORMAT_R16_TYPELESS)  return Format::R16_TYPELESS;
	if (format == DXGI_FORMAT_R16_FLOAT)  return Format::R16_FLOAT;
	if (format == DXGI_FORMAT_D16_UNORM)  return Format::D16_UNORM;
	if (format == DXGI_FORMAT_R16_UNORM)  return Format::R16_UNORM;
	if (format == DXGI_FORMAT_R16_UINT)  return Format::R16_UINT;
	if (format == DXGI_FORMAT_R16_SNORM)  return Format::R16_SNORM;
	if (format == DXGI_FORMAT_R16_SINT)  return Format::R16_SINT;
	if (format == DXGI_FORMAT_R8_TYPELESS)  return Format::R8_TYPELESS;
	if (format == DXGI_FORMAT_R8_UNORM)  return Format::R8_UNORM;
	if (format == DXGI_FORMAT_R8_UINT)  return Format::R8_UINT;
	if (format == DXGI_FORMAT_R8_SNORM)  return Format::R8_SNORM;
	if (format == DXGI_FORMAT_R8_SINT)  return Format::R8_SINT;
	if (format == DXGI_FORMAT_A8_UNORM)  return Format::A8_UNORM;
	if (format == DXGI_FORMAT_R1_UNORM)  return Format::R1_UNORM;
	if (format == DXGI_FORMAT_R9G9B9E5_SHAREDEXP)  return Format::R9G9B9E5_SHAREDEXP;
	if (format == DXGI_FORMAT_R8G8_B8G8_UNORM)  return Format::R8G8_B8G8_UNORM;
	if (format == DXGI_FORMAT_G8R8_G8B8_UNORM)  return Format::G8R8_G8B8_UNORM;
	if (format == DXGI_FORMAT_BC1_TYPELESS)  return Format::BC1_TYPELESS;
	if (format == DXGI_FORMAT_BC1_UNORM)  return Format::BC1_UNORM;
	if (format == DXGI_FORMAT_BC1_UNORM_SRGB)  return Format::BC1_UNORM_SRGB;
	if (format == DXGI_FORMAT_BC2_TYPELESS)  return Format::BC2_TYPELESS;
	if (format == DXGI_FORMAT_BC2_UNORM)  return Format::BC2_UNORM;
	if (format == DXGI_FORMAT_BC2_UNORM_SRGB)  return Format::BC2_UNORM_SRGB;
	if (format == DXGI_FORMAT_BC3_TYPELESS)  return Format::BC3_TYPELESS;
	if (format == DXGI_FORMAT_BC3_UNORM)  return Format::BC3_UNORM;
	if (format == DXGI_FORMAT_BC3_UNORM_SRGB)  return Format::BC3_UNORM_SRGB;
	if (format == DXGI_FORMAT_BC4_TYPELESS)  return Format::BC4_TYPELESS;
	if (format == DXGI_FORMAT_BC4_UNORM)  return Format::BC4_UNORM;
	if (format == DXGI_FORMAT_BC4_SNORM)  return Format::BC4_SNORM;
	if (format == DXGI_FORMAT_BC5_TYPELESS)  return Format::BC5_TYPELESS;
	if (format == DXGI_FORMAT_BC5_UNORM)  return Format::BC5_UNORM;
	if (format == DXGI_FORMAT_BC5_SNORM)  return Format::BC5_SNORM;
	if (format == DXGI_FORMAT_B5G6R5_UNORM)  return Format::B5G6R5_UNORM;
	if (format == DXGI_FORMAT_B5G5R5A1_UNORM)  return Format::B5G5R5A1_UNORM;
	if (format == DXGI_FORMAT_B8G8R8A8_UNORM)  return Format::B8G8R8A8_UNORM;
	if (format == DXGI_FORMAT_B8G8R8X8_UNORM)  return Format::B8G8R8X8_UNORM;
	if (format == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM)  return Format::R10G10B10_XR_BIAS_A2_UNORM;
	if (format == DXGI_FORMAT_B8G8R8A8_TYPELESS)  return Format::B8G8R8A8_TYPELESS;
	if (format == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)  return Format::B8G8R8A8_UNORM_SRGB;
	if (format == DXGI_FORMAT_B8G8R8X8_TYPELESS)  return Format::B8G8R8X8_TYPELESS;
	if (format == DXGI_FORMAT_B8G8R8X8_UNORM_SRGB)  return Format::B8G8R8X8_UNORM_SRGB;
	if (format == DXGI_FORMAT_BC6H_TYPELESS)  return Format::BC6H_TYPELESS;
	if (format == DXGI_FORMAT_BC6H_UF16)  return Format::BC6H_UF16;
	if (format == DXGI_FORMAT_BC6H_SF16)  return Format::BC6H_SF16;
	if (format == DXGI_FORMAT_BC7_TYPELESS)  return Format::BC7_TYPELESS;
	if (format == DXGI_FORMAT_BC7_UNORM)  return Format::BC7_UNORM;
	if (format == DXGI_FORMAT_BC7_UNORM_SRGB)  return Format::BC7_UNORM_SRGB;

	assert(false);
	return Format::UNKNOWN;
}

export D3D12_DESCRIPTOR_RANGE_TYPE to_native(DescriptorRange range)
{
	static constexpr D3D12_DESCRIPTOR_RANGE_TYPE natives[] =
	{
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV ,
		D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV ,
		D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER
	};

	return natives[static_cast<uint>(range)];
}

export D3D12_SHADER_VISIBILITY to_native(ShaderVisibility visibility)
{
	static constexpr D3D12_SHADER_VISIBILITY natives[] =
	{
		D3D12_SHADER_VISIBILITY_ALL,
		D3D12_SHADER_VISIBILITY_VERTEX,
		D3D12_SHADER_VISIBILITY_HULL,
		D3D12_SHADER_VISIBILITY_DOMAIN,
		D3D12_SHADER_VISIBILITY_GEOMETRY,
		D3D12_SHADER_VISIBILITY_PIXEL,
		D3D12_SHADER_VISIBILITY_AMPLIFICATION,
		D3D12_SHADER_VISIBILITY_MESH
	};

	return natives[static_cast<uint>(visibility)];
}

export D3D12_RESOURCE_FLAGS to_native(const ResFlags& flags)
{
	D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	if (!check(flags & ResFlags::ShaderResource))
	{
		result |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}

	if (check(flags & ResFlags::UnorderedAccess))
	{
		result |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	if (check(flags & ResFlags::RenderTarget))
	{
		result |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}

	if (check(flags & ResFlags::DepthStencil))
	{
		result |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}

	return result;
}


export ResFlags from_native(D3D12_RESOURCE_FLAGS flags)
{
	ResFlags result = ResFlags::None;

	if (!(flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE))
	{
		result |= ResFlags::ShaderResource;
	}

	if (flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
	{
		result |= ResFlags::UnorderedAccess;
	}

	if (flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
	{
		result |= ResFlags::RenderTarget;
	}

	if (flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		result |= ResFlags::DepthStencil;
	}

	return result;
}


export D3D12_VIEWPORT to_native(Viewport v)
{
	return D3D12_VIEWPORT{ v.pos.x,v.pos.y,v.size.x,v.size.y,v.depths.x,v.depths.y };
}

export CD3DX12_RESOURCE_DESC to_native(const ResourceDesc& desc)
{
	if (desc.is_buffer())
	{
		auto buffer_desc = desc.as_buffer();
		return CD3DX12_RESOURCE_DESC::Buffer(buffer_desc.SizeInBytes, to_native(desc.Flags));
	}
	else
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
	}
	assert(false);

	return CD3DX12_RESOURCE_DESC::Buffer(0, to_native(desc.Flags));
}


export namespace cereal
{
	template<class Archive>
	void serialize(Archive& ar, D3D12_DEPTH_STENCILOP_DESC& g, const unsigned int)
	{
		//	ar & g.DefaultValue;
		ar& NVPG(StencilDepthFailOp);
		ar& NVPG(StencilFailOp);
		ar& NVPG(StencilFunc);
		ar& NVPG(StencilPassOp);
	}

	template<class Archive>
	void serialize(Archive& ar, DXGI_SAMPLE_DESC& g, const unsigned int)
	{
		//	ar & g.DefaultValue;
		ar& NVPG(Count);
		ar& NVPG(Quality);
	}


	template<class Archive>
	void serialize(Archive& ar, D3D12_RESOURCE_DESC& g, const unsigned int)
	{
		ar& NVPG(Dimension);
		ar& NVPG(Alignment);
		ar& NVPG(Width);
		ar& NVPG(Height);
		ar& NVPG(DepthOrArraySize);
		ar& NVPG(MipLevels);
		ar& NVPG(Format);
		ar& NVPG(SampleDesc);
		ar& NVPG(Layout);
		ar& NVPG(Flags);
	}

	template<class Archive>
	void serialize(Archive& ar, D3D12_AUTO_BREADCRUMB_NODE& g, const unsigned int)
	{
		std::wstring CommandListDebugName = g.pCommandListDebugNameW ? g.pCommandListDebugNameW : L"";
		std::wstring CommandQueueDebugName = g.pCommandQueueDebugNameW ? g.pCommandQueueDebugNameW : L"";
		ar& NVP(CommandListDebugName);
		ar& NVP(CommandQueueDebugName);
		if (g.pLastBreadcrumbValue && *g.pLastBreadcrumbValue < g.BreadcrumbCount - 1)
		{
			ar& NP("FAILED", true);
		}


		std::vector<D3D12_AUTO_BREADCRUMB_OP> ops;
		ops.assign(g.pCommandHistory, g.pCommandHistory + g.BreadcrumbCount);

		UINT offset = g.pLastBreadcrumbValue ? (*g.pLastBreadcrumbValue) : g.BreadcrumbCount;

		for (UINT i = 0; i < ops.size(); i++)
		{
			auto op = ops[i];
			if (i <= offset) ar& NP("DONE", op);
			if (i == offset + 1) ar& NP("FAILED", op);
			if (i > offset + 1) ar& NP("WAITING", op);

		}
	}
}