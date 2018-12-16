#pragma once
#include "pch.h"
#include "Samplers.h"

namespace DX12
{
    namespace Samplers
    {
		/*
         D3D12_FILTER Filter;
		D3D12_TEXTURE_ADDRESS_MODE AddressU;
		D3D12_TEXTURE_ADDRESS_MODE AddressV;
		D3D12_TEXTURE_ADDRESS_MODE AddressW;
		FLOAT MipLODBias;
		UINT MaxAnisotropy;
		D3D12_COMPARISON_FUNC ComparisonFunc;
		FLOAT BorderColor[4];
		FLOAT MinLOD;
		FLOAT MaxLOD; */
		SamplerDesc SamplerPointClampDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_POINT ,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerPointBorderDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_POINT ,D3D12_TEXTURE_ADDRESS_MODE_BORDER,D3D12_TEXTURE_ADDRESS_MODE_BORDER,D3D12_TEXTURE_ADDRESS_MODE_BORDER,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,0.f,0.f,0.f,0.f,0,D3D12_FLOAT32_MAX };

		SamplerDesc SamplerLinearWrapDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerLinearClampDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerLinearBorderDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_BORDER,D3D12_TEXTURE_ADDRESS_MODE_BORDER,D3D12_TEXTURE_ADDRESS_MODE_BORDER,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,0.f,0.f,0.f,0.f,0,D3D12_FLOAT32_MAX };
		
    	SamplerDesc SamplerAnisoWrapDesc = SamplerDesc{ D3D12_FILTER_ANISOTROPIC ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerShadowDesc = SamplerDesc{ D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT ,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,0.0f,16,D3D12_COMPARISON_FUNC_GREATER_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerShadowComparisonDesc = SamplerDesc{ D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
		SamplerDesc SamplerVolumeWrapDesc = SamplerDesc{ D3D12_FILTER_MIN_MAG_MIP_POINT ,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,D3D12_TEXTURE_ADDRESS_MODE_WRAP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
     
  	/*
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
		*/
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
		/*
		struct _samplers_initializer
		{
			_samplers_initializer()
			{
				Samplers::SamplerLinearWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				Samplers::SamplerAnisoWrapDesc.MaxAnisotropy = 8;


				Samplers::SamplerShadowDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
				Samplers::SamplerShadowDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
				Samplers::SamplerShadowDesc.set_adress(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);


				Samplers::SamplerLinearClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				Samplers::SamplerLinearClampDesc.set_adress(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);


				Samplers::SamplerVolumeWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;


				Samplers::SamplerPointClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				Samplers::SamplerPointClampDesc.set_adress(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);


				Samplers::SamplerLinearBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
				Samplers::SamplerLinearBorderDesc.set_adress(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
				Samplers::SamplerLinearBorderDesc.set_color({ 0.0f, 0.0f, 0.0f, 0.0f });


				Samplers::SamplerPointBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				Samplers::SamplerPointBorderDesc.set_adress(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
				Samplers::SamplerPointBorderDesc.set_color({ 0.0f, 0.0f, 0.0f, 0.0f });

				Samplers::SamplerShadowComparisonDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
				Samplers::SamplerShadowComparisonDesc.set_adress(D3D12_TEXTURE_ADDRESS_MODE_WRAP);
				Samplers::SamplerShadowComparisonDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			}

		}samplers_initializer;
		*/
	}




}