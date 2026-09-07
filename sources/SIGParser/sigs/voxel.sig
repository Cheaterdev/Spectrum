struct VoxelTilingParams
{
	uint4 voxels_per_tile;
	StructuredBuffer<int3> tiles;

	%{
		
uint3 get_voxel_pos(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / voxels_per_tile.x;
	uint3 tile_pos = GetTiles()[tile_index] * voxels_per_tile.xyz;

	uint3 tile_local_pos = dispatchID - int3(tile_index * voxels_per_tile.x, 0, 0);
	uint3 index = tile_pos + tile_local_pos;
	return index;
}
		
	}%
}

[Bind = DefaultLayout::Instance0]
struct VoxelInfo
{
	float4 min;
	float4 size;
	uint4 voxels_per_tile;
	uint4 voxel_tiles_count;
}


[Bind = DefaultLayout::Instance3]
struct Voxelization
{
	VoxelInfo info;
	
	RWTexture3D<float4> albedo;
	RWTexture3D<float4> normals;
	RWTexture3D<uint> visibility;
	
}


[Bind = DefaultLayout::Instance1]
struct VoxelScreen
{
	GBuffer gbuffer;

	Texture3D<float4> voxels;
	TextureCube<float4> tex_cube;

	# Nothing assigns these -- the history they want is not wired up yet, and
	# voxel_screen.hlsl / raytracing.hlsl sample them every frame regardless.
	# Unannotated they read descriptor index 0, i.e. an unrelated resource.
	[Auto = Texture_Null]
	Texture2D<float4> prev_gi;

	[Auto = Texture_Null]
	Texture2D<float> prev_frames;
	[Auto = Texture_Null]
	Texture2D<float> prev_depth;
}

[Bind = DefaultLayout::Instance2]
struct VoxelOutput
{
	RWTexture2D<float4> noise;
	RWTexture2D<float> frames;

	RWTexture2D<float4> DirAndPdf;
	 Texture2D<float2> blueNoise;

	# Raw (pre-history-lerp), YCoCg-packed indirect-GI signal for NRD
	# REBLUR_DIFFUSE -- see [[project-nrd-integration]] and PassNode
	# VoxelScreen's VoxelIndirectNoiseRaw.
	RWTexture2D<float4> noiseRaw;
}

[Bind = DefaultLayout::Instance2]
struct VoxelBlur
{
	Texture2D<float4> noisy_output;
	Texture2D<float4> hit_and_pdf;

	Texture2D<float4> prev_result;

	RWTexture2D<float4> screen_result;
	RWTexture2D<float4> gi_result;

	# Set from g_indirect_denoiser (see [[project-nrd-integration]]): nonzero
	# when NRD_IndirectCombine will add the REBLUR-denoised indirect term
	# onto ResultTexture instead -- this shader still computes gi_result
	# (VoxelIndirectFiltered, kept fresh for a Legacy fallback) but skips its
	# own additive contribution to screen_result to avoid double-adding.
	uint skip_composite;
}

[Bind = DefaultLayout::Instance2]
struct VoxelUpscale
{
	Texture2D<float4> tex_downsampled;
	Texture2D<float4> tex_gi_prev;
	Texture2D<float> tex_depth_prev;
}


[Bind = DefaultLayout::Instance1]
struct VoxelVisibility
{	
	Texture3D<uint> visibility;
	AppendStructuredBuffer<uint4> visible_tiles;	
}


[Bind = DefaultLayout::Instance1]
struct VoxelMipMap
{
	
	RWTexture3D<float4> OutMips[3];
	Texture3D<float4> SrcMip;

	VoxelTilingParams params;
}

[Bind = DefaultLayout::Instance1]
struct VoxelCopy
{
	RWTexture3D<float4> Target[2];
	Texture3D<float4> Source[2];

	VoxelTilingParams params;
}

[Bind = DefaultLayout::Instance1]
struct VoxelZero
{

	RWTexture3D<float4> Target;

	VoxelTilingParams params;
}

[Bind = DefaultLayout::Instance1]
struct VoxelLighting
{
	
	RWTexture3D<float4> output;
	
	Texture3D<float4> albedo;
	Texture3D<float4> normals;
	Texture3D<float4> lower;

	TextureCube<float4> tex_cube;

	VSMShadowLookup vsm;

	VoxelTilingParams params;
}


[Bind = DefaultLayout::Instance1]
struct VoxelDebug
{
	GBuffer gbuffer;
	Texture3D<float4> volume;
}

ComputePSO Lighting
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxelgi/voxel_lighting;

	[rename = SECOND_BOUNCE]
	[CS, nullable]
	define SecondBounce;
}


ComputePSO VoxelDownsample
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxelgi/voxel_mipmap;

	[rename = COUNT]
	[CS]
	define Count = {1, 2, 3};
}



ComputePSO VoxelCopy
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxelgi/voxel_copy;
}


ComputePSO VoxelZero
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxelgi/voxel_zero;
}

ComputePSO VoxelVisibility
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxelgi/voxel_visibility;
}

ComputePSO VoxelIndirectFilter
{
	root = DefaultLayout;

	[EntryPoint = PS]
	compute = voxelgi/voxel_screen_blur;

	[rename = ENABLE_BLUR]
	[CS, nullable]
	define Blur;

	[rename = REFLECTION]
	[CS, nullable]
	define Reflection;


}

GraphicsPSO VoxelReflectionHi
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxelgi/voxel_screen;

	[EntryPoint = PS]
	pixel = voxelgi/voxel_screen_reflection;

	enable_stencil = true;
	enable_depth = false;
	stencil_func = ALWAYS;
	stencil_pass_op = Keep;

	rtv = {R16G16B16A16_FLOAT};
	ds = D24_UNORM_S8_UINT;
		blend = { Additive };

	stencil_read_mask = 2;
	stencil_write_mask = 2;
}


GraphicsPSO VoxelReflectionUpsample
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxelgi/voxel_screen;

	[EntryPoint = PS_resize]
	pixel = voxelgi/voxel_screen_reflection;

	enable_stencil = true;
	enable_depth = false;
	stencil_func = ALWAYS;
	stencil_pass_op = Keep;

	rtv = {R16G16B16A16_FLOAT};
	ds = D24_UNORM_S8_UINT;
	blend = { Additive };

	stencil_read_mask = 2;
	stencil_write_mask = 2;
}


GraphicsPSO VoxelIndirectHi
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxelgi/voxel_screen;

	[EntryPoint = PS]
	pixel = voxelgi/voxel_screen;

	enable_stencil = true;
	enable_depth = false;
	stencil_func = EQUAL;
	stencil_pass_op = Keep;

	blend = { Additive , None};
	rtv = { R16G16B16A16_FLOAT, R11G11B10_FLOAT};
	ds = D24_UNORM_S8_UINT;

	stencil_read_mask = 1;
	stencil_write_mask = 1;
}


ComputePSO VoxelIndirectLow
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxelgi/voxel_screen;
}



GraphicsPSO VoxelIndirectUpsample
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxelgi/voxel_screen;

	[EntryPoint = PS_Resize]
	pixel = voxelgi/voxel_screen;

	enable_stencil = true;
	enable_depth = false;
	stencil_func = EQUAL;
	stencil_pass_op = Keep;

	blend = { Additive , None};
	rtv = { R16G16B16A16_FLOAT, R11G11B10_FLOAT};
	ds = D24_UNORM_S8_UINT;

	stencil_read_mask = 1;
	stencil_write_mask = 1;
}


GraphicsPSO VoxelDebug
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxelgi/voxel_screen;

	[EntryPoint = Debug]
	pixel = voxelgi/voxel_screen_debug;

	rtv = {R16G16B16A16_FLOAT};
	enable_depth = false;
}



[Bind = DefaultLayout::Instance2]
struct DenoiserDownsample
{
	Texture2D<float4> color;
	Texture2D<float> depth;
}


GraphicsPSO DenoiserDownsample
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = denoiser/denoiser_downsample;

	[EntryPoint = PS]
	pixel = denoiser/denoiser_downsample;

	rtv = { R16G16B16A16_FLOAT, R16_FLOAT };
}


struct TilingParams
{
	StructuredBuffer<uint2> tiles;

	%{
		
uint2 get_pixel_pos(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / 32;
	uint2 tile_pos = GetTiles()[tile_index] * 32;

	uint2 tile_local_pos = dispatchID.xy - int3(tile_index * 32, 0, 0);
	uint2 index = tile_pos + tile_local_pos;
	return index;
}
		
	}%
}

[Bind = DefaultLayout::Instance2]
struct DenoiserHistoryFix
{
	Texture2D<float4> color;
	Texture2D<float> frames;

	RWTexture2D<float4> target;
}


[Bind = FrameLayout::PassData]
struct TilingPostprocess
{
	TilingParams tiling;
}



ComputePSO DenoiserHistoryFix
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = denoiser/denoiser_history_fix;
}


[Bind = DefaultLayout::Instance2]
struct FrameClassification
{

	Texture2D<float> frames;

	AppendStructuredBuffer<uint2> hi;
	AppendStructuredBuffer<uint2> low;
}

ComputePSO FrameClassification
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = gbuffer/frame_classification;
}

[Bind = DefaultLayout::Instance2]
struct FrameClassificationInitDispatch
{
	StructuredBuffer<uint> hi_counter;	
	StructuredBuffer<uint> low_counter;	

	RWStructuredBuffer<DispatchArguments> hi_dispatch_data;
	RWStructuredBuffer<DispatchArguments> low_dispatch_data;

}


ComputePSO FrameClassificationInitDispatch
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = gbuffer/frame_classification_init_dispatch;
}

[Bind = DefaultLayout::Instance0]
struct ReflectionCombine
{
	GBuffer gbuffer;
	Texture2D<float4> reflection;

	RWTexture2D<float4> target;

	# Same as RTXCombine's unpack_reflection (see [[project-nrd-integration]]):
	# nonzero when `reflection` above is REBLUR SPECULAR's packed output
	# (RTXReflectionDenoised) and needs an unpack; zero for the legacy
	# FFX-denoised VoxelReflectionNoise, never YCoCg-encoded.
	uint unpack_reflection;
}


ComputePSO ReflectionCombine
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = postprocess/reflection_combine;
}

# Computes full lighting on its own -- NOT a composite over ResultTexture
# (which carries VSM/PSSM's shadow baked into it already, the exact problem
# that made combining RTXShadowNoise into it ambiguous). Direct sun lighting
# here uses the SAME formula every RTX hit point in the engine uses
# (MyClosestHitShader, UniversalMaterialRaytracing.hlsl: albedo * NdotL *
# sun_visibility), with RTXShadowNoise standing in for that hit-shader's own
# traced sun_vis. Reflection/indirect GI add on top. Output goes to
# ResultTextureRTXNoise -- ResultTexture is untouched, not even read.
[Bind = DefaultLayout::Instance0]
struct RTXCombine
{
	GBuffer gbuffer;
	Texture2D<float4> reflection;
	Texture2D<float4> indirect;
	Texture2D<float4> shadow;

	RWTexture2D<float4> target;

	# Set from g_indirect_denoiser (see [[project-nrd-integration]]): nonzero
	# when `indirect` above is REBLUR's YCoCg+hitdist-packed output
	# (RTXIndirectDenoised) and needs REBLUR_BackEnd_UnpackRadianceAndNormHitDist;
	# zero when it's the legacy VCT pipeline's plain VoxelIndirectFiltered,
	# which was never YCoCg-encoded.
	uint unpack_indirect;
	# Same as unpack_indirect, but for `reflection` above / g_reflection_denoiser
	# (RTXReflectionDenoised, packed, vs raw RTXReflectionNoise).
	uint unpack_reflection;
}

ComputePSO RTXCombine
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = rtx/rtx_combine;
}


PassNode GBufferDownsampler
{
	GBuffer gbuffer;
}

PassNode VoxelDebug
{
	GBuffer gbuffer;
	[Write] Texture VoxelDebug;
	Texture3D VoxelLighted;
}

[Compute]
PassNode VoxelScreen
{
	GBuffer gbuffer;
	Texture ResultTexture;
	Texture3D VoxelLighted;
	[Write] Texture VoxelFramesCount;
	[Write] Texture VoxelIndirectNoise;
	# Raw (pre-history-lerp) signal for NRD REBLUR_DIFFUSE -- see
	# [[project-nrd-integration]]. Always written (both the RTX and
	# non-RTX trace paths), unused when g_indirect_source picks the RTX
	# reference signal instead.
	[Write] Texture VoxelIndirectNoiseRaw;
	[Write] Texture VoxelIndirectFiltered;
	# Previous-frame view of VoxelIndirectFiltered (history-prev, adopted
	# resource). READ-ONLY by design — the FrameGraph skips alloc/free
	# scheduling for a history prev and assumes it is never written. A fresh
	# prev (first frame, no carried history) is handled shader-side, not by
	# clearing it here.
	Texture VoxelIndirectFilteredPrev;
	TextureCube sky_cubemap_filtered;
	Texture BlueNoise;
	[Write] StructuredBuffer<DispatchArguments> VoxelScreen_hi;
	[Write] StructuredBuffer<DispatchArguments> VoxelScreen_low;
	[Write] StructuredBuffer<uint2> VoxelScreen_low_data;
	[Write] StructuredBuffer<uint2> VoxelScreen_hi_data;
}

[Compute]
PassNode VoxelCombine
{
	GBuffer gbuffer;
	[Write] Texture ResultTexture;
	[Write] Texture VoxelFramesCount;
	Texture VoxelIndirectNoise;
	[Write] Texture VoxelIndirectFiltered;
	# Previous-frame view of VoxelIndirectFiltered (history-prev, adopted resource).
	Texture VoxelIndirectFilteredPrev;
	TextureCube sky_cubemap_filtered;
	StructuredBuffer<DispatchArguments> VoxelScreen_hi;
	StructuredBuffer<DispatchArguments> VoxelScreen_low;
	StructuredBuffer<uint2> VoxelScreen_low_data;
	StructuredBuffer<uint2> VoxelScreen_hi_data;
}

[Compute]
PassNode ScreenReflection
{
	GBuffer gbuffer;
	[Write] Texture VoxelReflectionNoise;
	# Raw (pre-ReflectionDenoiser_Reproject) YCoCg-packed signal for NRD
	# REBLUR_SPECULAR -- see [[project-nrd-integration]]. Always written,
	# unused when g_reflection_source picks the RTX reference signal instead.
	[Write] Texture VoxelReflectionNoiseRaw;
	[Write] Texture noise_dir_pdf;
	TextureCube sky_cubemap_filtered;
	Texture BlueNoise;
	Texture3D VoxelLighted;
	StructuredBuffer<DispatchArguments> VoxelScreen_hi;
	StructuredBuffer<DispatchArguments> VoxelScreen_low;
	StructuredBuffer<uint2> VoxelScreen_low_data;
	StructuredBuffer<uint2> VoxelScreen_hi_data;
}

# RTX-only sibling of ScreenReflection, for DLSS-RR's consumption
# (UpscalingDLSSRR.sig). A genuinely independent pass, not a variant of
# ScreenReflection's dispatch: dispatches its own standalone raygen
# (MyRaygenShaderReflectionRTXOnly, raytracing.hlsl) that shares no code with
# MyRaygenShaderReflection -- one ray per pixel, blue-noise-jittered
# direction, no VoxelInfo/voxel cone trace, no voxel-cone fallback on miss.
# Untouched: ScreenReflection itself, its raygen, and the old FFX
# ReflectionDenoiser_Reproject/ReflCombine pipeline VoxelReflectionNoise
# feeds. [Static]: nothing here is VoxelGI-private -- gated purely on RTX
# support and DLSS-RR availability, both globally accessible.
[Static]
[Compute]
PassNode ReflectionRTX
{
	GBuffer gbuffer;
	Texture BlueNoise;

	[Write] Texture RTXReflectionNoise;
	[Write] Texture RTXReflectionDirPdf;
}

# RTX-only reference shadow, sibling of ReflectionRTX/IndirectRTX -- same
# rationale (see ReflectionRTX's doc comment above). One ray per pixel
# toward a jittered direction within the sun's angular disk (same technique
# ShadowRaygenShader's own 16-sample reference uses, just 1 sample instead
# of 16 -- genuinely noisy, not averaged), no temporal history. Composited
# by RTXCombine below; RTXShadow itself (Bend/FFX hybrid denoiser) is
# unaffected -- this is a separate signal, not a replacement.
[Static]
[Compute]
PassNode ShadowRTX
{
	GBuffer gbuffer;

	[Write] Texture RTXShadowNoise;
}

# RTX-only reference diffuse GI, sibling of ReflectionRTX/ShadowRTX. One ray
# per pixel, GGX-importance-sampled hemisphere direction, no voxel-cone-trace
# fallback on miss, no temporal history. Composited by RTXCombine below;
# VoxelScreen's Indirect dispatch is unaffected -- this is a separate signal,
# not a replacement.
[Static]
[Compute]
PassNode IndirectRTX
{
	GBuffer gbuffer;
	Texture BlueNoise;

	[Write] Texture RTXIndirectNoise;
}

# Composites the FFX-denoised voxel/RTX reflection blend (ScreenReflection +
# ReflectionDenoiser_Reproject) onto ResultTexture. Does NOT run when
# RTXCombine will (see RTXCombine's own doc comment below) -- exactly one of
# the two composites onto ResultTexture each frame.
[Compute]
PassNode ReflCombine
{
	GBuffer gbuffer;
	[Write] Texture ResultTexture;
	Texture VoxelReflectionNoise;
	# NRD REBLUR_SPECULAR's denoised output -- alternative reflection input
	# selected by g_reflection_denoiser instead of VoxelReflectionNoise (same
	# resource NRD_REBLUR_Execute creates for RTXCombine, produced regardless
	# of upscaler -- see [[project-nrd-integration]]).
	Texture RTXReflectionDenoised;
}

# DLSS-RR-active counterpart to ReflCombine, but NOT a composite over
# ResultTexture -- it computes full lighting itself (direct sun + reflection
# + indirect GI, see RTXCombine.hlsl) and writes ResultTextureRTXNoise, the
# ColorIn UpscalingDLSSRR tags. ResultTexture (and whatever VSM/PSSM baked
# into it) plays no part here at all. Gated identically to its three
# producers -- runs instead of ReflCombine whenever DLSS-RR is the active
# upscaler.
[Static]
[Compute]
# RTXIndirectDenoised (REBLUR_DIFFUSE's output, NRD_REBLUR_Execute --
# see [[project-nrd-integration]]) replaces the raw noisy RTXIndirectNoise
# signal here -- straight swap, no toggle.
PassNode RTXCombine
{
	GBuffer gbuffer;
	Texture RTXReflectionNoise;
	# NRD REBLUR_SPECULAR's denoised output -- alternative reflection input
	# selected by g_reflection_denoiser instead of RTXReflectionNoise.
	Texture RTXReflectionDenoised;
	Texture RTXIndirectDenoised;
	# Legacy VCT full pipeline's output -- alternative indirect input
	# selected by g_indirect_denoiser instead of RTXIndirectDenoised.
	Texture VoxelIndirectFiltered;
	Texture RTXShadowNoise;

	[Write] Texture ResultTextureRTXNoise;
}

PassNode Voxelize
{
	[Write] Texture VoxelAlbedo;
	[Write] Texture VoxelNormal;
	[Write] Texture VoxelAlbedoStatic;
	[Write] Texture VoxelNormalStatic;
	[Write] Texture VoxelAlbedoDynamic;
	[Write] Texture VoxelNormalDynamic;
}

[Compute]
PassNode Lighting
{
	Texture VSM_Atlas;
	Texture VSM_PageTable;
	StructuredBuffer<Camera> VSM_PageCameras;
	[Write] Texture3D VoxelLighted;
	Texture3D VoxelAlbedo;
	Texture3D VoxelNormal;
	TextureCube sky_cubemap_filtered;
	Texture VoxelAlbedoStatic;
	Texture VoxelNormalStatic;
	Texture VoxelAlbedoDynamic;
	Texture VoxelNormalDynamic;
}

[Compute]
PassNode Mipmapping
{
	[Write] Texture3D VoxelLighted;
}
