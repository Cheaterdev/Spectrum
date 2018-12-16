#pragma once


namespace DX12
{
    namespace Samplers
    {
        struct SamplerDesc: public D3D12_SAMPLER_DESC
        {
		//	SamplerDesc();

			void set_adress(D3D12_TEXTURE_ADDRESS_MODE AddressMode);

			void set_color(vec4 color);


        };
        extern SamplerDesc SamplerLinearWrapDesc;
        extern SamplerDesc SamplerAnisoWrapDesc;
        extern SamplerDesc SamplerShadowDesc;
        extern SamplerDesc SamplerLinearClampDesc;
        extern SamplerDesc SamplerVolumeWrapDesc;
        extern SamplerDesc SamplerPointClampDesc;
        extern SamplerDesc SamplerPointBorderDesc;
        extern SamplerDesc SamplerLinearBorderDesc;
		extern SamplerDesc SamplerShadowComparisonDesc;

    }
}