#include "../autogen/FGTSamplerProbe.h"

// Texture filtering with every input fixed (Test.HAL.ixx, Sampler_FilteringProbe).
// Nine 128-wide columns, one per sampler/view (8: exact trilinear by hand); down the image the footprint
// grows from below mip 0 to past the last mip, across a column the UVs drift
// (and, in the anisotropic columns, the anisotropy ratio grows). SampleGrad,
// not Sample: the gradients are given, so nothing depends on rasterization.
[numthreads(8, 8, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
	const uint2 size = GetFGTSamplerProbe().GetSize();
	if (any(id.xy >= size))
		return;

	Texture2D<float4> tex      = GetFGTSamplerProbe().GetTex();
	Texture2D<float4> tex_srgb = GetFGTSamplerProbe().GetTex_srgb();

	uint2 dims;
	tex.GetDimensions(dims.x, dims.y);

	const uint  column = id.x / 128;
	const float u      = (float(id.x % 128) + 0.31) / 128.0;
	const float v      = (float(id.y) + 0.27) / float(size.y);

	// Footprint in texels, as a mip level from -1 to past the top of the chain.
	const float lod   = lerp(-1.0, 11.0, v);
	const float texel = exp2(lod) / float(dims.x);
	const float2 uv   = float2(u * 1.7 + v * 0.37, v * 3.1 + u * 0.13);

	float2 ddx_uv = float2(texel, 0);
	float2 ddy_uv = float2(0, texel);
	if (column == 4 || column == 7)
	{
		// Anisotropic: stretch one axis up to 16x and rotate the footprint.
		const float ratio = lerp(1.0, 16.0, u);
		const float c = cos(0.6), s = sin(0.6);
		ddx_uv = float2(c, s) * texel;
		ddy_uv = float2(-s, c) * texel * ratio;
	}

	float4 result;
	switch (column)
	{
	case 0:  result = tex.SampleGrad(linearSampler, uv, ddx_uv, ddy_uv); break;
	case 1:  result = tex_srgb.SampleGrad(linearSampler, uv, ddx_uv, ddy_uv); break;
	case 2:  result = tex_srgb.SampleGrad(linearClampSampler, uv, ddx_uv, ddy_uv); break;
	case 3:  result = tex_srgb.SampleGrad(pointClampSampler, uv, ddx_uv, ddy_uv); break;
	case 4:  result = tex_srgb.SampleGrad(anisoBordeSampler, uv, ddx_uv, ddy_uv); break;
	case 5:  result = tex_srgb.SampleGrad(linearBorderBlackSampler, uv * 1.4 - 0.2, ddx_uv, ddy_uv); break;
	case 6:  result = tex_srgb.SampleLevel(linearSampler, uv, max(lod, 0.0)); break;
	case 8:
	{
		// Exact trilinear by hand, for column 6 to be checked against: at
		// whole levels the sampler only filters bilinearly, which drivers
		// don't shortcut, so the blend between levels is the textbook one.
		const float l = max(lod, 0.0);
		result = lerp(tex_srgb.SampleLevel(linearSampler, uv, floor(l)),
		              tex_srgb.SampleLevel(linearSampler, uv, floor(l) + 1), frac(l));
		break;
	}
	case 7:  result = tex.SampleGrad(anisoBordeSampler, uv, ddx_uv, ddy_uv); break;
	}

	GetFGTSamplerProbe().GetOutput()[id.xy] = result;
}
