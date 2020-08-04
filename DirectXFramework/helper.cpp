#include "pch.h"




UINT BitsPerPixel(DXGI_FORMAT fmt)
{
    switch (fmt)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 128;

        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 96;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return 64;

        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            return 32;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            return 16;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
            return 8;

        case DXGI_FORMAT_R1_UNORM:
            return 1;

        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            return 4;

        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC6H_UF16:
        case DXGI_FORMAT_BC6H_SF16:
        case DXGI_FORMAT_BC7_TYPELESS:
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return 8;

        default:
            assert(FALSE); // unhandled format
            return 0;
    }
}
UINT get_default_mapping(DXGI_FORMAT fmt)
{
    return D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    switch (fmt)
    {
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
            return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;

        default:
            return D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    }
}
DXGI_FORMAT to_typeless(DXGI_FORMAT fmt)
{
	
		switch (fmt)
		{
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:

			return DXGI_FORMAT_R8G8B8A8_TYPELESS;


		default:
			return fmt;
		}
}

DXGI_FORMAT to_linear(DXGI_FORMAT fmt)
{
	switch (fmt)
	{
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:

		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;


	default:
		return fmt;
	}
}

DXGI_FORMAT to_srv(DXGI_FORMAT fmt)
{
    switch (fmt)
    {
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_R32_FLOAT;

        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_R16_FLOAT;

        case DXGI_FORMAT_R8_TYPELESS:
            return DXGI_FORMAT_R8_UNORM;

        default:
            return fmt;
    }
}
DXGI_FORMAT to_dsv(DXGI_FORMAT fmt)
{
    switch (fmt)
    {
        case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_D32_FLOAT;

        case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_D16_UNORM;

        case DXGI_FORMAT_R8_TYPELESS:
            return DXGI_FORMAT_R8_TYPELESS; //oops!

        default:
            return fmt;
    }
}

bool is_shader_visible(DXGI_FORMAT fmt)
{

	switch (fmt)
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

bool operator<(const D3D11_SAMPLER_DESC& l, const D3D11_SAMPLER_DESC& r)
{
    //return true;
    OP(AddressU,
       OP(Filter,
          OP(AddressU,
             OP(AddressV,
                OP(AddressW,
                   OP(MipLODBias,
                      OP(MaxAnisotropy,
                         OP(ComparisonFunc,
                            OP(BorderColor[0],
                               OP(BorderColor[1],
                                  OP(BorderColor[2],
                                     OP(BorderColor[3],
                                        OP(MinLOD,
                                           OP(MaxLOD, return false;)
                                          )
                                       )
                                    )
                                 )
                              )
                           )
                        )
                     )
                  )
               )
            )
         )
      );
}

bool operator<(const D3D11_DEPTH_STENCIL_DESC& l, const D3D11_DEPTH_STENCIL_DESC& r)
{
    //return true;
    OP(DepthEnable,
       OP(DepthWriteMask,
          OP(DepthFunc,
             OP(StencilEnable,
                OP(StencilReadMask,
                   OP(StencilWriteMask,
                      OP(FrontFace.StencilFailOp,
                         OP(FrontFace.StencilDepthFailOp,
                            OP(FrontFace.StencilPassOp,
                               OP(FrontFace.StencilFunc,
                                  OP(BackFace.StencilFailOp,
                                     OP(BackFace.StencilDepthFailOp,
                                        OP(BackFace.StencilPassOp,
                                           OP(BackFace.StencilFunc, return false;)
                                          )
                                       )
                                    )
                                 )
                              )
                           )
                        )
                     )
                  )
               )
            )
         )
      );
}



bool operator<(const D3D11_RASTERIZER_DESC& l, const D3D11_RASTERIZER_DESC& r)
{
    OP(FillMode,
       OP(CullMode,
          OP(FrontCounterClockwise,
             OP(DepthBias,
                OP(DepthBiasClamp,
                   OP(SlopeScaledDepthBias,
                      OP(DepthClipEnable,
                         OP(ScissorEnable,
                            OP(MultisampleEnable,
                               OP(AntialiasedLineEnable,
                                  return false;)
                              )
                           )
                        )
                     )
                  )
               )
            )
         )
      );
}



bool operator<(const D3D11_RENDER_TARGET_BLEND_DESC& l, const D3D11_RENDER_TARGET_BLEND_DESC& r)
{
    OP(BlendEnable,
       OP(SrcBlend,
          OP(DestBlend,
             OP(BlendOp,
                OP(SrcBlendAlpha,
                   OP(DestBlendAlpha,
                      OP(BlendOpAlpha,
                         OP(RenderTargetWriteMask,
                            return false;)
                        )
                     )
                  )
               )
            )
         )
      );
}


bool operator<(const D3D11_BLEND_DESC& l, const D3D11_BLEND_DESC& r)
{
    OP(AlphaToCoverageEnable,
       OP(IndependentBlendEnable,
          OP(RenderTarget[0],
             OP(RenderTarget[1],
                OP(RenderTarget[2],
                   OP(RenderTarget[3],
                      OP(RenderTarget[4],
                         OP(RenderTarget[5],
                            OP(RenderTarget[6],
                               OP(RenderTarget[7],
                                  return false;)
                              )
                           )
                        )
                     )
                  )
               )
            )
         )
      );
}

bool operator==(const D3D11_RENDER_TARGET_BLEND_DESC& l, const D3D11_RENDER_TARGET_BLEND_DESC& r)
{
    return l.BlendEnable == r.BlendEnable &&
           l.BlendOp == r.BlendOp &&
           l.BlendOpAlpha == r.BlendOpAlpha &&
           l.DestBlend == r.DestBlend &&
           l.DestBlendAlpha == r.DestBlendAlpha &&
           l.RenderTargetWriteMask == r.RenderTargetWriteMask &&
           l.SrcBlend == r.SrcBlend &&
           l.SrcBlendAlpha == r.SrcBlendAlpha;
}

void boost::serialization::serialize(simple_log_archive& ar, D3D11_BLEND_DESC& g, const unsigned int version)
{
    ar& g.AlphaToCoverageEnable;
    ar& g.IndependentBlendEnable;
}
