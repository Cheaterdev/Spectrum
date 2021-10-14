#pragma once

import d3d12_types;
import Vectors;

namespace DX12
{
    namespace Samplers
    {
        struct SamplerDesc: public D3D12_SAMPLER_DESC
        {
			void set_adress(D3D12_TEXTURE_ADDRESS_MODE AddressMode);

			void set_color(vec4 color);
        };

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