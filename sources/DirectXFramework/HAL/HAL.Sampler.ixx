export module HAL.Sampler;

import Vectors;

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

    enum class ComparisonFunc : uint
    {
        NEVER,
        LESS,
        EQUAL,
        LESS_EQUAL,
        GREATER,
        NOT_EQUAL,
        GREATER_EQUAL,
        ALWAYS,
        NONE,
        MIN,
        MAX
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
