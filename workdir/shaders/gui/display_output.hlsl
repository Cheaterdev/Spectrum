#ifndef DISPLAY_OUTPUT_HLSL
#define DISPLAY_OUTPUT_HLSL

#include "../autogen/DisplayOutput.h"

float3 srgb_to_linear(float3 c)
{
	return select(c <= 0.04045, c / 12.92, pow((c + 0.055) / 1.055, 2.4));
}

// UI art and colors are authored sRGB-encoded; the swapchain is scRGB
// (linear, 1.0 = 80 nits). SDR white lands at ui_scale.
float4 ui_output(float4 srgb)
{
	return float4(srgb_to_linear(saturate(srgb.rgb)) * GetDisplayOutput().GetUi_scale(), srgb.a);
}

// Already-linear content (the tonemapped viewport): 1.0 = SDR white, values
// above 1 are HDR highlights and pass through.
float4 ui_output_linear(float4 c)
{
	return float4(max(c.rgb, 0) * GetDisplayOutput().GetUi_scale(), c.a);
}

#endif
