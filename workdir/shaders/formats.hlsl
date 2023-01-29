 #define UNKNOWN 0
 #define R32G32B32A32_TYPELESS 1
 #define R32G32B32A32_FLOAT 2
 #define R32G32B32A32_UINT 3
 #define R32G32B32A32_SINT 4
 #define R32G32B32_TYPELESS 5
 #define R32G32B32_FLOAT 6
 #define R32G32B32_UINT 7
 #define R32G32B32_SINT 8
 #define R16G16B16A16_TYPELESS 9
 #define R16G16B16A16_FLOAT 10
 #define R16G16B16A16_UNORM 11
 #define R16G16B16A16_UINT 12
 #define R16G16B16A16_SNORM 13
 #define R16G16B16A16_SINT 14
 #define R32G32_TYPELESS 15
 #define R32G32_FLOAT 16
 #define R32G32_UINT 17
 #define R32G32_SINT 18
 #define R32G8X24_TYPELESS 19
 #define D32_FLOAT_S8X24_UINT 20
 #define R32_FLOAT_X8X24_TYPELESS 21
 #define X32_TYPELESS_G8X24_UINT 22
 #define R10G10B10A2_TYPELESS 23
 #define R10G10B10A2_UNORM 24
 #define R10G10B10A2_UINT 25
 #define R11G11B10_FLOAT 26
 #define R8G8B8A8_TYPELESS 27
 #define R8G8B8A8_UNORM 28
 #define R8G8B8A8_UNORM_SRGB 29
 #define R8G8B8A8_UINT 30
 #define R8G8B8A8_SNORM 31
 #define R8G8B8A8_SINT 32
 #define R16G16_TYPELESS 33
 #define R16G16_FLOAT 34
 #define R16G16_UNORM 35
 #define R16G16_UINT 36
 #define R16G16_SNORM 37
 #define R16G16_SINT 38
 #define R32_TYPELESS 39
 #define D32_FLOAT 40
 #define R32_FLOAT 41
 #define R32_UINT 42
 #define R32_SINT 43
 #define R24G8_TYPELESS 44
 #define D24_UNORM_S8_UINT 45
 #define R24_UNORM_X8_TYPELESS 46
 #define X24_TYPELESS_G8_UINT 47
 #define R8G8_TYPELESS 48
 #define R8G8_UNORM 49
 #define R8G8_UINT 50
 #define R8G8_SNORM 51
 #define R8G8_SINT 52
 #define R16_TYPELESS 53
 #define R16_FLOAT 54
 #define D16_UNORM 55
 #define R16_UNORM 56
 #define R16_UINT 57
 #define R16_SNORM 58
 #define R16_SINT 59
 #define R8_TYPELESS 60
 #define R8_UNORM 61
 #define R8_UINT 62
 #define R8_SNORM 63
 #define R8_SINT 64
 #define A8_UNORM 65
 #define R1_UNORM 66
 #define R9G9B9E5_SHAREDEXP 67
 #define R8G8_B8G8_UNORM 68
 #define G8R8_G8B8_UNORM 69
 #define BC1_TYPELESS 70
 #define BC1_UNORM 71
 #define BC1_UNORM_SRGB 72
 #define BC2_TYPELESS 73
 #define BC2_UNORM 74
 #define BC2_UNORM_SRGB 75
 #define BC3_TYPELESS 76
 #define BC3_UNORM 77
 #define BC3_UNORM_SRGB 78
 #define BC4_TYPELESS 79
 #define BC4_UNORM 80
 #define BC4_SNORM 81
 #define BC5_TYPELESS 82
 #define BC5_UNORM 83
 #define BC5_SNORM 84
 #define B5G6R5_UNORM 85
 #define B5G5R5A1_UNORM 86
 #define B8G8R8A8_UNORM 87
 #define B8G8R8X8_UNORM 88
 #define R10G10B10_XR_BIAS_A2_UNORM 89
 #define B8G8R8A8_TYPELESS 90
 #define B8G8R8A8_UNORM_SRGB 91
 #define B8G8R8X8_TYPELESS 92
 #define B8G8R8X8_UNORM_SRGB 93
 #define BC6H_TYPELESS 94
 #define BC6H_UF16 95
 #define BC6H_SF16 96
 #define BC7_TYPELESS 97
 #define BC7_UNORM 98
 #define BC7_UNORM_SRGB 99
 #define AYUV 100
 #define Y410 101
 #define Y416 102
 #define NV12 103
 #define P010 104
 #define P016 105
 //#define 420_OPAQUE 106
 #define YUY2 107
 #define Y210 108
 #define Y216 109
 #define NV11 110
 #define AI44 111
 #define IA44 112
 #define P8 113
 #define A8P8 114
 #define B4G4R4A4_UNORM 115
 #define P208 130
 #define V208 131
 #define V408 132
#if Format == UNKNOWN
#define type void
#endif
#if Format == R32G32B32A32_TYPELESS
#define type void
#endif
#if Format == R32G32B32A32_FLOAT
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R32G32B32A32_UINT
#define type uint4
#define type4 uint4
#define format_count 4
#endif
#if Format == R32G32B32A32_SINT
#define type int4
#define type4 int4
#define format_count 4
#endif
#if Format == R32G32B32_TYPELESS
#define type void
#endif
#if Format == R32G32B32_FLOAT
#define type float3
#define type4 float4
#define format_count 3
#endif
#if Format == R32G32B32_UINT
#define type uint3
#define type4 uint4
#define format_count 2
#endif
#if Format == R32G32B32_SINT
#define type int3
#define type4 int4
#define type4 int4
#define format_count 3
#endif
#if Format == R16G16B16A16_TYPELESS
#define type void
#endif
#if Format == R16G16B16A16_FLOAT
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R16G16B16A16_UNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R16G16B16A16_UINT
#define type uint4
#define type4 uint4
#define format_count 4
#endif
#if Format == R16G16B16A16_SNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R16G16B16A16_SINT
#define type int4
#define type4 int4
#define format_count 4
#endif
#if Format == R32G32_TYPELESS
#define type void
#endif
#if Format == R32G32_FLOAT
#define type float2
#define type4 float4
#define format_count 2
#endif
#if Format == R32G32_UINT
#define type uint2
#define type4 uint4
#define format_count 2
#endif
#if Format == R32G32_SINT
#define type int2
#define type4 int4
#define type4 int4
#define format_count 2
#endif
#if Format == R32G8X24_TYPELESS
#define type void
#endif
#if Format == D32_FLOAT_S8X24_UINT
#define type void
#endif
#if Format == R32_FLOAT_X8X24_TYPELESS
#define type void
#endif
#if Format == X32_TYPELESS_G8X24_UINT
#define type void
#endif
#if Format == R10G10B10A2_TYPELESS
#define type void
#endif
#if Format == R10G10B10A2_UNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R10G10B10A2_UINT
#define type uint4
#define type4 uint4
#define format_count 4
#endif
#if Format == R11G11B10_FLOAT
#define type float3
#define type4 float4
#define format_count 3
#endif
#if Format == R8G8B8A8_TYPELESS
#define type void
#endif
#if Format == R8G8B8A8_UNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R8G8B8A8_UNORM_SRGB
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R8G8B8A8_UINT
#define type uint4
#define type4 uint4
#define format_count 4
#endif
#if Format == R8G8B8A8_SNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R8G8B8A8_SINT
#define type int4
#define type4 int4
#define format_count 4
#endif
#if Format == R16G16_TYPELESS
#define type void
#endif
#if Format == R16G16_FLOAT
#define type float2
#define type4 float4
#define format_count 2
#endif
#if Format == R16G16_UNORM
#define type float2
#define type4 float4
#define format_count 2
#endif
#if Format == R16G16_UINT
#define type uint2
#define type4 uint4
#define format_count 2
#endif
#if Format == R16G16_SNORM
#define type float2
#define type4 float4
#define type4 float4
#define format_count 2
#endif
#if Format == R16G16_SINT
#define type int2
#define type4 int4
#define type4 int4
#define format_count 2
#endif
#if Format == R32_TYPELESS
#define type void
#endif
#if Format == D32_FLOAT
#define type void
#endif
#if Format == R32_FLOAT
#define type float
#define type4 float4
#define format_count 1
#endif
#if Format == R32_UINT
#define type uint
#define type4 uint4
#endif
#if Format == R32_SINT
#define type int
#define type4 int4
#define format_count 1
#endif
#if Format == R24G8_TYPELESS
#define type void
#endif
#if Format == D24_UNORM_S8_UINT
#define type void
#endif
#if Format == R24_UNORM_X8_TYPELESS
#define type void
#endif
#if Format == X24_TYPELESS_G8_UINT
#define type void
#endif
#if Format == R8G8_TYPELESS
#define type void
#endif
#if Format == R8G8_UNORM
#define type float2
#define type4 float4
#define format_count 2
#endif
#if Format == R8G8_UINT
#define type uint2
#define type4 uint4
#define format_count 2
#endif
#if Format == R8G8_SNORM
#define type float2
#define type4 float4
#define format_count 2
#endif
#if Format == R8G8_SINT
#define type int2
#define type4 int4
#define format_count 2
#endif
#if Format == R16_TYPELESS
#define type void
#endif
#if Format == R16_FLOAT
#define type float
#define type4 float4
#define format_count 1
#endif
#if Format == D16_UNORM
#define type void
#endif
#if Format == R16_UNORM
#define type float
#define type4 float4
#define format_count 1
#endif
#if Format == R16_UINT
#define type uint4
#define type4 uint4
#define format_count 1
#endif
#if Format == R16_SNORM
#define type float
#define type4 float4
#define format_count 1
#endif
#if Format == R16_SINT
#define type int
#define type4 int4
#define format_count 1
#endif
#if Format == R8_TYPELESS
#define type void
#endif
#if Format == R8_UNORM
#define type float
#define type4 float4
#define format_count 1
#endif
#if Format == R8_UINT
#define type uint4
#define type4 uint4
#define format_count 1
#endif
#if Format == R8_SNORM
#define type float
#define type4 float4
#define format_count 1
#endif
#if Format == R8_SINT
#define type int
#define type4 int4
#define format_count 1
#endif
#if Format == A8_UNORM
#define type float
#define type4 float4
#define format_count 1
#endif
#if Format == R1_UNORM
#define type float
#define type4 float4
#define format_count 1
#endif
#if Format == R9G9B9E5_SHAREDEXP
#define type uint4
#define type4 uint4
#define format_count 4
#endif
#if Format == R8G8_B8G8_UNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == G8R8_G8B8_UNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == BC1_TYPELESS
#define type void
#endif
#if Format == BC1_UNORM
#define type void
#endif
#if Format == BC1_UNORM_SRGB
#define type void
#endif
#if Format == BC2_TYPELESS
#define type void
#endif
#if Format == BC2_UNORM
#define type void
#endif
#if Format == BC2_UNORM_SRGB
#define type void
#endif
#if Format == BC3_TYPELESS
#define type void
#endif
#if Format == BC3_UNORM
#define type void
#endif
#if Format == BC3_UNORM_SRGB
#define type void
#endif
#if Format == BC4_TYPELESS
#define type void
#endif
#if Format == BC4_UNORM
#define type void
#endif
#if Format == BC4_SNORM
#define type void
#endif
#if Format == BC5_TYPELESS
#define type void
#endif
#if Format == BC5_UNORM
#define type void
#endif
#if Format == BC5_SNORM
#define type void
#endif
#if Format == B5G6R5_UNORM
#define type float3
#define type4 float4
#define format_count 3
#endif
#if Format == B5G5R5A1_UNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == B8G8R8A8_UNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == B8G8R8X8_UNORM
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == R10G10B10_XR_BIAS_A2_UNORM
#define type void
#endif
#if Format == B8G8R8A8_TYPELESS
#define type void
#endif
#if Format == B8G8R8A8_UNORM_SRGB
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == B8G8R8X8_TYPELESS
#define type void
#endif
#if Format == B8G8R8X8_UNORM_SRGB
#define type float4
#define type4 float4
#define format_count 4
#endif
#if Format == BC6H_TYPELESS
#define type void
#endif
#if Format == BC6H_UF16
#define type void
#endif
#if Format == BC6H_SF16
#define type void
#endif
#if Format == BC7_TYPELESS
#define type void
#endif
#if Format == BC7_UNORM
#define type void
#endif
#if Format == BC7_UNORM_SRGB
#define type void
#endif