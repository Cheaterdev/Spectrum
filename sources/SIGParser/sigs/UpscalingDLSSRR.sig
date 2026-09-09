# DLSS Ray Reconstruction. Per NVIDIA's actual integration guide
# (https://github.com/NVIDIA-RTX/Streamline/blob/main/docs/ProgrammingGuideDLSS_RR.md,
# not just slGetFeatureRequirements, which under-reports the real required
# set): RR "completely overrides DLSS Super Resolution" for the whole frame
# -- it replaces UpscalingDLSS outright, exactly like SR/FSR/SMAA's existing
# mutual exclusion (see UpscalingDLSS.cpp / FSR.cpp's g_upscaling_enabled
# checks) -- while ALSO wanting reflection-specific hint buffers most SR-only
# integrations wouldn't have: Albedo, NormalRoughness, SpecularHitDistance,
# SpecularAlbedo (this engine tags hit-distance instead of the doc's
# alternative, SpecularMotionVectors, since RTXReflectionNoise already has
# it -- see ReflectionRTX, voxel.sig). SpecularAlbedo (F0) is derived by
# NormalRoughnessRepack below from GBuffer_Albedo's rgb=albedo/w=metallic
# via lerp(0.04, albedo, metallic).
#
# No ComputePSO for the RR evaluate itself: like DLSS-SR, it is an opaque call
# into the Streamline SDK operating on tagged resources via the command list
# (see UpscalingDLSSRR.cpp's PassDefault<Passes::UpscalingDLSSRR>::render),
# not a shader this engine dispatches directly.

# Decodes GBuffer_Normals' best-fit-compressed normal (see FrameData.sig's
# compress_normals()) back to an unpacked world-space normal, and repacks it
# with roughness into one Streamline-compatible buffer for the
# kBufferTypeNormalRoughness tag -- Streamline can't consume the compressed
# encoding directly. Also derives SpecularAlbedo (F0) from GBuffer_Albedo in
# the same dispatch, since both are per-pixel GBuffer-derived and RR wants
# them together. Both are pure GBuffer-derived material properties, not
# reflection-specific, so this only gates on DLSS-RR being available -- see
# ReflectionRTX (voxel.sig) for the actual reflection-signal input, gated
# separately on RTX support.
[Bind = DefaultLayout::Instance2]
struct NormalRoughnessRepackParams
{
	Texture2D<float4>   GBuffer_Normals;
	Texture2D<float4>   GBuffer_Albedo;
	RWTexture2D<float4> Output;
	RWTexture2D<float4> SpecularAlbedoOutput;
}

ComputePSO NormalRoughnessRepack
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = gbuffer/normal_roughness_repack;
}

PassNode NormalRoughnessRepack
{
	[Always = Read] Texture GBuffer_Normals;
	[Always = Read] Texture GBuffer_Albedo;

	[Write] Texture NormalRoughness;
	[Write] Texture SpecularAlbedo;
}

[Static]
PassNode UpscalingDLSSRR
{
	Texture GBuffer_Depth;
	Texture GBuffer_Speed;
	Texture GBuffer_Albedo;
	Texture NormalRoughness;
	Texture SpecularAlbedo;
	Texture RTXReflectionNoise;

	# ColorIn actually comes from here (RTXCombine's output, voxel.sig), not
	# from ResultTexture below -- see UpscalingDLSSRR.cpp's render(). The
	# [Recreate] pairing on ResultTexture is still needed to produce
	# ResultTextureNew (the pass's actual output), so that field stays even
	# though its own value is no longer read.
	Texture ResultTextureRTXNoise;

	[Write] [Recreate = ResultTextureNew]
	Texture ResultTexture;
}
