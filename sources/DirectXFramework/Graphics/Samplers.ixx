export module Graphics:Samplers;
import :Types;
import Math;

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

	namespace Blends
	{
		extern RenderTarget None;
		extern RenderTarget Additive;
		extern RenderTarget AlphaBlend;
	}
}

namespace Graphics
{
	namespace Samplers
	{

		SamplerDesc SamplerPointClampDesc = SamplerDesc{ Filter::POINT, Filter::POINT, Filter::POINT,
			TextureAddressMode::CLAMP, TextureAddressMode::CLAMP , TextureAddressMode::CLAMP,	0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };
		SamplerDesc SamplerPointBorderDesc = SamplerDesc{ Filter::POINT, Filter::POINT, Filter::POINT,
			TextureAddressMode::BORDER, TextureAddressMode::BORDER , TextureAddressMode::BORDER,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };

		SamplerDesc SamplerLinearWrapDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::LINEAR,
			TextureAddressMode::WRAP, TextureAddressMode::WRAP , TextureAddressMode::WRAP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };
		SamplerDesc SamplerLinearClampDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::LINEAR,
			TextureAddressMode::CLAMP, TextureAddressMode::CLAMP , TextureAddressMode::CLAMP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };
		SamplerDesc SamplerLinearBorderDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::LINEAR,
			TextureAddressMode::BORDER, TextureAddressMode::BORDER , TextureAddressMode::BORDER,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };

		SamplerDesc SamplerAnisoWrapDesc = SamplerDesc{ Filter::ANISOTROPIC, Filter::ANISOTROPIC, Filter::ANISOTROPIC,
			TextureAddressMode::WRAP, TextureAddressMode::WRAP , TextureAddressMode::WRAP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };
		SamplerDesc SamplerAnisoBorderDesc = SamplerDesc{ Filter::ANISOTROPIC, Filter::ANISOTROPIC, Filter::ANISOTROPIC,
			TextureAddressMode::WRAP, TextureAddressMode::WRAP , TextureAddressMode::BORDER,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };

		SamplerDesc SamplerShadowDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::POINT, TextureAddressMode::CLAMP, TextureAddressMode::CLAMP ,
			TextureAddressMode::CLAMP,0.0f,16, ComparisonFunc::NONE, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };
		SamplerDesc SamplerShadowComparisonDesc = SamplerDesc{ Filter::LINEAR, Filter::LINEAR, Filter::LINEAR, TextureAddressMode::CLAMP, TextureAddressMode::CLAMP ,
			TextureAddressMode::CLAMP,0.0f,16, ComparisonFunc::LESS_EQUAL, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };
		SamplerDesc SamplerVolumeWrapDesc = SamplerDesc{ Filter::POINT, Filter::POINT, Filter::POINT, TextureAddressMode::WRAP, TextureAddressMode::WRAP ,
			TextureAddressMode::WRAP,0.0f,16, ComparisonFunc::LESS_EQUAL, float4(1,1,1,1), 0, std::numeric_limits<float>::max() };
		
	};

	namespace Blends
	{
		RenderTarget None = RenderTarget{ false, Blend::One, Blend::One, Blend::One, Blend::One };
		RenderTarget Additive = RenderTarget{ true, Blend::One, Blend::One, Blend::One, Blend::One };
		RenderTarget AlphaBlend = RenderTarget{ true, Blend::InvSrcAlpha, Blend::SrcAlpha, Blend::One, Blend::One };
	}

}