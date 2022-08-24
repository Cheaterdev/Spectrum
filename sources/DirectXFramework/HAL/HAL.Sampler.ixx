export module HAL.Sampler;

import HAL.Types;
import Math;

export namespace HAL
{
    enum class Filter: uint
    {
        POINT,
        LINEAR,
        ANISOTROPIC
    };

    enum class TextureAddressMode : uint
    {
        WRAP,
        MIRROR,
        CLAMP,
        BORDER,
        MIRROR_ONCE
    };


    struct SamplerDesc
    {
        Filter             MinFilter;
        Filter             MagFilter;
        Filter             MipFilter;

        TextureAddressMode AddressU;
        TextureAddressMode AddressV;
        TextureAddressMode AddressW;

        float              MipLODBias;
        uint               MaxAnisotropy;
        ComparisonFunc     ComparisonFunc;
        float4 BorderColor;
        float              MinLOD;
        float              MaxLOD;
    };
}
