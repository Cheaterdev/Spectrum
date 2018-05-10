#pragma once
#include "pch.h"
#include "Samplers.h"

namespace DX12
{
    namespace Samplers
    {

        SamplerDesc SamplerLinearWrapDesc;
        SamplerDesc SamplerAnisoWrapDesc;
        SamplerDesc SamplerShadowDesc;
        SamplerDesc SamplerLinearClampDesc;
        SamplerDesc SamplerVolumeWrapDesc;
        SamplerDesc SamplerPointClampDesc;
        SamplerDesc SamplerPointBorderDesc;
        SamplerDesc SamplerLinearBorderDesc;
		SamplerDesc SamplerShadowComparisonDesc;

		SamplerDesc::SamplerDesc()
		{
			Filter = D3D12_FILTER_ANISOTROPIC;
			AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			MipLODBias = 0.0f;
			MaxAnisotropy = 16;
			ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			BorderColor[0] = 1.0f;
			BorderColor[1] = 1.0f;
			BorderColor[2] = 1.0f;
			BorderColor[3] = 1.0f;
			MinLOD = 0.0f;
			MaxLOD = D3D12_FLOAT32_MAX;
		}

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
}