export module Samplers;

import d3d12;
import d3d12_types;
import Vectors;

export import Graphics.Types;

export namespace Graphics
{
    namespace Samplers
    {
        extern SamplerDesc SamplerLinearWrapDesc;
        extern SamplerDesc SamplerAnisoWrapDesc;
		extern SamplerDesc SamplerAnisoBorderDesc;
		extern SamplerDesc SamplerShadowDesc;
        extern SamplerDesc SamplerLinearClampDesc;
        extern SamplerDesc SamplerVolumeWrapDesc;
        extern SamplerDesc SamplerPointClampDesc;
        extern SamplerDesc SamplerPointBorderDesc;
        extern SamplerDesc SamplerLinearBorderDesc;
		extern SamplerDesc SamplerShadowComparisonDesc;

    }

    namespace Blend
    {
		extern RenderTarget None;
		extern RenderTarget Additive;
		extern RenderTarget AlphaBlend;
    }
}

module:private;


namespace Graphics
{
	namespace Samplers
	{

		SamplerDesc SamplerPointClampDesc = SamplerDesc{ Filter::POINT, Filter::POINT, Filter::POINT, TextureAddressMode::CLAMP, TextureAddressMode::CLAMP , TextureAddressMode::CLAMP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
		SamplerDesc SamplerPointBorderDesc = SamplerDesc{ Filter::POINT, Filter::POINT, Filter::POINT, TextureAddressMode::BORDER, TextureAddressMode::BORDER , TextureAddressMode::BORDER,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
	
		SamplerDesc SamplerLinearWrapDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::LINEAR, TextureAddressMode::WRAP, TextureAddressMode::WRAP , TextureAddressMode::WRAP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
		SamplerDesc SamplerLinearClampDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::LINEAR, TextureAddressMode::CLAMP, TextureAddressMode::CLAMP , TextureAddressMode::CLAMP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
		SamplerDesc SamplerLinearBorderDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::LINEAR, TextureAddressMode::BORDER, TextureAddressMode::BORDER , TextureAddressMode::BORDER,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
		
		SamplerDesc SamplerAnisoWrapDesc = SamplerDesc{ Filter::ANISOTROPIC, Filter::ANISOTROPIC, Filter::ANISOTROPIC, TextureAddressMode::WRAP, TextureAddressMode::WRAP , TextureAddressMode::WRAP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
		SamplerDesc SamplerAnisoBorderDesc = SamplerDesc{ Filter::ANISOTROPIC, Filter::ANISOTROPIC, Filter::ANISOTROPIC, TextureAddressMode::WRAP, TextureAddressMode::WRAP , TextureAddressMode::BORDER,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
	
		SamplerDesc SamplerShadowDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::POINT, TextureAddressMode::CLAMP, TextureAddressMode::CLAMP , TextureAddressMode::CLAMP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
		SamplerDesc SamplerShadowComparisonDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::LINEAR, TextureAddressMode::CLAMP, TextureAddressMode::CLAMP , TextureAddressMode::CLAMP,0.0f,16, ComparisonFunc::LESS_EQUAL, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX
		SamplerDesc SamplerVolumeWrapDesc = SamplerDesc{ Filter::POINT, Filter::POINT, Filter::POINT, TextureAddressMode::WRAP, TextureAddressMode::WRAP , TextureAddressMode::WRAP,0.0f,16, ComparisonFunc::LESS_EQUAL, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };// , D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, 0.0f, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, 1.f, 1.f, 1.f, 1.f, 0, D3D12_FLOAT32_MAX


	};
	


		/*
		SamplerDesc SamplerPointClampDesc = SamplerDesc{ ,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,D3D12_TEXTURE_ADDRESS_MODE_CLAMP,0.0f,16,D3D12_COMPARISON_FUNC_LESS_EQUAL,1.f,1.f,1.f,1.f,0,D3D12_FLOAT32_MAX };
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
		
	}*/

	namespace Blend
	{
		RenderTarget None = RenderTarget{ false, D3D12_BLEND::D3D12_BLEND_ONE, D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE };
		RenderTarget Additive = RenderTarget{ true, D3D12_BLEND::D3D12_BLEND_ONE, D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE };
		RenderTarget AlphaBlend = RenderTarget{ true, D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND::D3D12_BLEND_SRC_ALPHA , D3D12_BLEND::D3D12_BLEND_ONE , D3D12_BLEND::D3D12_BLEND_ONE };

	}



}