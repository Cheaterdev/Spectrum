# Prototype only (see [[project-nrd-integration]]): can NRD's own vendored
# shader source compile against this engine's SIG/bindless reflection instead
# of a hand-built root signature? Uses Clear.cs.hlsl (workdir/shaders/nrd/) --
# the smallest NRD kernel: 3 constants, 1 RWTexture2D output, no inputs, no
# samplers -- via a macro shim (workdir/shaders/nrd/sig_clear.hlsl) that
# redirects NRD.hlsli's NRD_CONSTANTS_START/NRD_CONSTANT/NRD_OUTPUTS_START/
# NRD_OUTPUT macros at this struct's generated accessors instead of raw
# register()s. Struct name MUST be "Clear_Constants" -- it has to match
# NRD_CONSTANTS_START( Clear_Constants ) in Clear.resources.hlsli exactly,
# since the shim macro token-pastes that name into Get##name().
#
# Reinstated for debugging (see [[project-nrd-integration]]): removing this
# PSO correlated with a wrap->clamp sampler bug on mesh rendering/material
# previews going away, but no causal mechanism was found -- NRD_Clear_Test's
# samplers are baked as isolated D3D12_STATIC_SAMPLER_DESC entries in its own
# root signature (no shared heap), and DefaultLayout's generated files were
# byte-identical with/without it. Brought back at user's request to debug
# directly rather than treat the correlation as proof.
[Bind = DefaultLayout::Instance2]
struct Clear_Constants
{
	float gDebug;
	float gViewZScale;
	float gDenoisingRange;
	RWTexture2D<float4> gOut;
}

ComputePSO NRD_Clear_Test
{
	root = DefaultLayout;

	[EntryPoint = main]
	compute = nrd/sig_clear;
}
