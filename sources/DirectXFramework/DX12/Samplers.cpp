#pragma once
#include "pch.h"
#include "Samplers.h"

namespace DX12
{
    namespace Samplers
    {
		SamplerDesc SamplerPointClampDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_POINT ,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerPointBorderDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_POINT ,D3D12_TEXTURE_ADDRESS_MODE_BORDER,D3D12_TEXTURE_ADDRESS_MODE_BORDER,D3D12_TEXTURE_ADDRESS_MODE_BORDER,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,0.f,0.f,0.f,0.f,0,D3D12_FLOAT32_MAX };

		SamplerDesc SamplerLinearWrapDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerLinearClampDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerLinearBorderDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_BORDER,D3D12_TEXTURE_ADDRESS_MODE_BORDER,D3D12_TEXTURE_ADDRESS_MODE_BORDER,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,0.f,0.f,0.f,0.f,0,D3D12_FLOAT32_MAX };
		
    	SamplerDesc SamplerAnisoWrapDesc = SamplerDesc{ D3D12_FILTER_ANISOTROPIC ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerAnisoBorderDesc = SamplerDesc{ D3D12_FILTER_ANISOTROPIC ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_BORDER,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerShadowDesc = SamplerDesc{ D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT ,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,0.0f,16,D3D12_COMPARISON_FUNC_GREATER_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerShadowComparisonDesc = SamplerDesc{ D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerVolumeWrapDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_POINT ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
     
		void SamplerDesc::set_adress(D3D12_TEXTURE_ADDRESS_MODE AddressMode)
		{
			AddressU = AddressMode;
			AddressV = AddressMode;
			AddressW = AddressMode;
		}

		void SamplerDesc::set_color(vec4 color)
		{
			BorderColor[0] = color.x;
			BorderColor[1] = color.y;
			BorderColor[2] = color.z;
			BorderColor[3] = color.w;
		}
	
	}

	namespace Blend
	{
		RenderTarget None = RenderTarget{ false, D3D12_BLEND::D3D12_BLEND_ONE, D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE };
		RenderTarget Additive = RenderTarget{ true, D3D12_BLEND::D3D12_BLEND_ONE, D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE };
		RenderTarget AlphaBlend = RenderTarget{ true, D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND::D3D12_BLEND_SRC_ALPHA , D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE };

	}



}