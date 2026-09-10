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

	# Packed IN_DIFF/SPEC_RADIANCE_HITDIST for NRD (PackForReblurDiffuse/
	# PackForReblurSpecular), written by MyRaygenShader/MyRaygenShaderReflection
	# (voxel-cone-traced source, see [[project-nrd-integration]]).
	RWTexture2D<float4> noiseRaw;
}

# Minimal depth+normal pair for MyRaygenShaderIndirectRTXHalfRes -- deliberately
# NOT the full GBuffer/VoxelScreen struct: this dispatch only has
# GBuffer_HalfDepth/HalfNormals, and populating a synthetic GBuffer with the
# rest left default risks an unbound-descriptor read (the same #939 class of
# bug documented elsewhere in this .sig) for channels the shader never
# touches but the table still declares.
[Bind = DefaultLayout::Instance4]
struct IndirectRTXHalfGBuffer
{
	Texture2D<float> depth;
	Texture2D<float4> normals;
}

# Read-side inputs for MyRaygenShaderIndirectRTXOnly's Low-tile shortcut: the
# half-res trace's packed output (bilinear-sampled directly -- REBLUR's
# packed radiance is a linear/YCoCg transform of RGB, so this is equivalent
# to sampling then re-packing, and NRD's own internal passes already
# resample this exact packed shape the same way) and the resolved per-tile
# Hi/Low flag (TileClassifyData's tile_flags) this pixel's tile belongs to.
[Bind = DefaultLayout::Instance3]
struct IndirectRTXUpscale
{
	Texture2D<float4> noiseHalf;
	Texture2D<uint> tileFlags;
}

# Read-side inputs for MyRaygenShaderReflectionRTXOnly's Low-tile shortcut --
# sibling of IndirectRTXUpscale, with a second half-res texture (dirPdfHalf,
# ReflectionRTX's own extra output) and a second tile-flag texture: this
# pixel is only skipped when BOTH the geometric tile (tileFlags,
# TileClassifyData's tile_flags) AND the roughness/metallic tile
# (roughnessTileFlags, TileClassifyData's tile_roughness_flags) say Low --
# either one alone forces a fresh trace, since a geometric edge makes the
# half-res buffer untrustworthy regardless of material, and a glossy+
# metallic surface needs real detail regardless of how flat it is.
#
# Instance5 -- raytracing.hlsl now has one struct per DefaultLayout instance
# slot (0-5, VoxelInfo/VoxelScreen/VoxelOutput/IndirectRTXUpscale/
# IndirectRTXHalfGBuffer/this), all six spoken for; a future addition to
# this file needs to fold into an existing struct rather than declare a new
# one (see IndirectRTXHalfGBuffer's own comment for why co-included structs
# can't share an instance).
[Bind = DefaultLayout::Instance5]
struct ReflectionRTXUpscale
{
	Texture2D<float4> noiseHalf;
	Texture2D<float4> dirPdfHalf;
	Texture2D<uint> tileFlags;
	Texture2D<uint> roughnessTileFlags;
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



[Bind = DefaultLayout::Instance0]
struct ReflectionCombine
{
	GBuffer gbuffer;
	Texture2D<float4> reflection;

	RWTexture2D<float4> target;
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
}

ComputePSO RTXCombine
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = rtx/rtx_combine;
}


# See TileClassifyData's own comment (pssm.sig) for the algorithm. [Static]
# and listed in MainPipeline (test.sig), same as IndirectRTX -- this used to
# be a runtime-wired add_library_pass with no actual call site left calling
# it (dead: the pass never ran, before or after this rewrite), stateless
# enough that [Static] + PassDefault<Passes::GBufferDownsampler> is the
# right shape, matching every other pass here.
[Static]
[Compute]
PassNode GBufferDownsampler
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment. GBuffer_Quality is intentionally NOT [Always] here either --
	# it still needs its own manual builder.need() in setup() (VoxelGIGraph.
	# cpp), same reasoning as before flattening (this is the one pass that
	# actually reads it).
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;

	# GBuffer_TempColor is the scratch target MipMapGenerator::generate_quality
	# needs -- a plain top-level field, not a PassView GBuffer leaf (see
	# pssm.sig's own comment on why those moved out).
	[Always = RenderTarget] [Size = ViewportContext::frame_size] [Format = R8G8_UNORM]
	Texture GBuffer_TempColor;

	# Raw [Size]: no grammar support for arithmetic, so the half-res transform
	# is pasted as a literal C++ expression -- SIGParser doesn't interpret it,
	# it's exactly the same (size+1)/2 the hand-written code used to compute.
	[Always = UnorderedAccess] [Size = `ivec2((builder.graph->get_context<Table::ViewportContext>().frame_size.x + 1) / 2, (builder.graph->get_context<Table::ViewportContext>().frame_size.y + 1) / 2)`] [Format = R32_FLOAT]
	Texture GBuffer_HalfDepth;
	[Always = UnorderedAccess] [Size = `ivec2((builder.graph->get_context<Table::ViewportContext>().frame_size.x + 1) / 2, (builder.graph->get_context<Table::ViewportContext>().frame_size.y + 1) / 2)`] [Format = R8G8B8A8_UNORM]
	Texture GBuffer_HalfNormals;

	# Worst case: every tile lands in the same bucket -- size each list for
	# the full tile count, same reasoning as VSM's own tile-classification
	# lists. `true` is the counted flag: GPU-appended (AppendStructuredBuffer),
	# needs a real counter.
	[Always = UnorderedAccess] [Size = `(size_t)Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.x, 8) * Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.y, 8), true`]
	StructuredBuffer<uint2> TileClassifyHi;
	[Always = UnorderedAccess] [Size = `(size_t)Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.x, 8) * Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.y, 8), true`]
	StructuredBuffer<uint2> TileClassifyLow;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R8_UINT] Texture TileClassifyMask;
	[Always = UnorderedAccess] [Size = `ivec2(Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.x, 8), Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.y, 8))`] [Format = R8_UINT]
	Texture TileClassifyTiles;

	[Always = UnorderedAccess] [Size = `(size_t)Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.x, 8) * Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.y, 8), true`]
	StructuredBuffer<uint2> TileRoughnessHi;
	[Always = UnorderedAccess] [Size = `(size_t)Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.x, 8) * Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.y, 8), true`]
	StructuredBuffer<uint2> TileRoughnessLow;
	[Always = UnorderedAccess] [Size = `ivec2(Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.x, 8), Math::DivideByMultiple(builder.graph->get_context<Table::ViewportContext>().frame_size.y, 8))`] [Format = R8_UINT]
	Texture TileRoughnessTiles;
}

PassNode VoxelDebug
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;
	[Always = RenderTarget] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture VoxelDebug;
	[Always = Read] Texture3D VoxelLighted;
}

# Always-on cheap base layer for ReflectionRTX's Low-tile pixels -- sibling
# of IndirectRTXHalf. Same trace as ReflectionRTX's Hi-tile path, over
# GBuffer_HalfDepth/HalfNormals.
[Static]
[Compute]
PassNode ReflectionRTXHalf
{
	[Always = Read] Texture GBuffer_HalfDepth;
	[Always = Read] Texture GBuffer_HalfNormals;
	[Always = Read] Texture BlueNoise;
	# Read-only, never bound through this PassNode's own Slots:: struct --
	# purely to force CubeMapEnviromentProcessor to run before this pass, so
	# FrameInfo.GetSky() (a separate global slot, sampled by the miss shader
	# via TraceReflection -> ColorPass) is actually populated instead of
	# reading an unbound/null cubemap. Same pattern RTXColorPass already
	# uses for its own miss shader, see its comment (this file).
	[Always = Read] TextureCube sky_cubemap_filtered;
	[Always = Read] TextureCube sky_cubemap_filtered_diffuse;

	[Always = UnorderedAccess] [Size = `ivec2((builder.graph->get_context<Table::ViewportContext>().frame_size.x + 1) / 2, (builder.graph->get_context<Table::ViewportContext>().frame_size.y + 1) / 2)`] [Format = R16G16B16A16_FLOAT]
	Texture RTXReflectionNoiseHalf;
	[Always = UnorderedAccess] [Size = `ivec2((builder.graph->get_context<Table::ViewportContext>().frame_size.x + 1) / 2, (builder.graph->get_context<Table::ViewportContext>().frame_size.y + 1) / 2)`] [Format = R16G16B16A16_FLOAT]
	Texture RTXReflectionDirPdfHalf;
}

# Reflection reference signal for both RTXCombine (DLSS-RR) and ReflCombine
# (FSR/DLSS) -- dispatches its own standalone raygen
# (MyRaygenShaderReflectionRTXOnly, raytracing.hlsl): one ray per pixel,
# blue-noise-jittered direction, no VoxelInfo/voxel cone trace. Denoised by
# NRD REBLUR_SPECULAR (NRD_REBLUR_Execute, see [[project-nrd-integration]]),
# the only reflection denoiser now. [Static]: nothing here is VoxelGI-private
# -- gated purely on RTX support and DLSS-RR availability, both globally
# accessible.
#
# Tile-classified like IndirectRTX (see its own comment above), gated on
# BOTH the geometric tile flag AND the roughness/metallic tile flag -- see
# ReflectionRTXUpscale's own comment for why both matter here specifically.
[Static]
[Compute]
PassNode ReflectionRTX
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;
	[Always = Read] Texture BlueNoise;
	[Always = Read] Texture RTXReflectionNoiseHalf;
	[Always = Read] Texture RTXReflectionDirPdfHalf;
	[Always = Read] Texture TileClassifyTiles;
	[Always = Read] Texture TileRoughnessTiles;
	# See ReflectionRTXHalf's own comment on the same two fields.
	[Always = Read] TextureCube sky_cubemap_filtered;
	[Always = Read] TextureCube sky_cubemap_filtered_diffuse;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture RTXReflectionNoise;
	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture RTXReflectionDirPdf;
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
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture RTXShadowNoise;
}

# Always-on cheap base layer for IndirectRTX's Low-tile pixels: same trace,
# same importance sampling, just over GBuffer_HalfDepth/HalfNormals instead
# of full res (a quarter the rays). See IndirectRTXUpscale's own comment
# (this file) for how IndirectRTX consumes RTXIndirectNoiseHalf.
[Static]
[Compute]
PassNode IndirectRTXHalf
{
	[Always = Read] Texture GBuffer_HalfDepth;
	[Always = Read] Texture GBuffer_HalfNormals;
	[Always = Read] Texture BlueNoise;
	# See ReflectionRTXHalf's own comment on the same two fields.
	[Always = Read] TextureCube sky_cubemap_filtered;
	[Always = Read] TextureCube sky_cubemap_filtered_diffuse;

	[Always = UnorderedAccess] [Size = `ivec2((builder.graph->get_context<Table::ViewportContext>().frame_size.x + 1) / 2, (builder.graph->get_context<Table::ViewportContext>().frame_size.y + 1) / 2)`] [Format = R16G16B16A16_FLOAT]
	Texture RTXIndirectNoiseHalf;
}

# Indirect-GI reference signal, sibling of ReflectionRTX/ShadowRTX. One ray
# per pixel, GGX-importance-sampled hemisphere direction, no voxel-cone-trace
# fallback on miss, no temporal history. Denoised by NRD REBLUR_DIFFUSE
# (NRD_REBLUR_Execute, see [[project-nrd-integration]]), the only indirect
# denoiser now, then composited by RTXCombine/ReflCombine below.
#
# Tile-classified: a Low-tile pixel (TileClassifyTiles, from
# GBufferDownsampler) skips its own TraceRay entirely and bilinear-samples
# IndirectRTXHalf's output instead (see raytracing.hlsl's
# MyRaygenShaderIndirectRTXOnly) -- first consumer of the generic tile
# system built for exactly this (TileClassifyData, pssm.sig). Only Hi tiles
# pay for a fresh ray.
[Static]
[Compute]
PassNode IndirectRTX
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;
	[Always = Read] Texture BlueNoise;
	[Always = Read] Texture RTXIndirectNoiseHalf;
	[Always = Read] Texture TileClassifyTiles;
	# See ReflectionRTXHalf's own comment on the same two fields.
	[Always = Read] TextureCube sky_cubemap_filtered;
	[Always = Read] TextureCube sky_cubemap_filtered_diffuse;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture RTXIndirectNoise;
}

# Composites NRD's denoised reflection onto ResultTexture. Does NOT run when
# RTXCombine will (see RTXCombine's own doc comment below) -- exactly one of
# the two composites onto ResultTexture each frame.
[Compute]
PassNode ReflCombine
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;
	[Always = UnorderedAccess] Texture ResultTexture;
	# NRD REBLUR_SPECULAR's denoised output (NRD_REBLUR_Execute, see
	# [[project-nrd-integration]]) -- the only reflection denoiser now.
	[Always = Read] Texture RTXReflectionDenoised;
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
# RTXIndirectDenoised/RTXReflectionDenoised (REBLUR_DIFFUSE/SPECULAR's
# output, NRD_REBLUR_Execute -- see [[project-nrd-integration]]) are the
# only indirect/reflection inputs now -- no raw/legacy alternative.
PassNode RTXCombine
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;
	# Raw, not REBLUR-denoised -- DLSS-RR does its own reconstruction/
	# denoising on this exact signal (its ColorIn tag), so feeding it
	# NRD-denoised data would be a redundant double-denoise. NRD isn't even
	# running when this pass does (NRD_REBLUR_Execute is gated off under
	# DLSS-RR) -- see NRD_GBufferPack's own comment (nrd_sig_test.sig).
	[Always = Read] Texture RTXReflectionNoise;
	[Always = Read] Texture RTXIndirectNoise;
	[Always = Read] Texture RTXShadowNoise;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture ResultTextureRTXNoise;
}

PassNode Voxelize
{
	[Always = UnorderedAccess] Texture VoxelAlbedo;
	[Always = UnorderedAccess] Texture VoxelNormal;
	[Always = UnorderedAccess] Texture VoxelAlbedoStatic;
	[Always = UnorderedAccess] Texture VoxelNormalStatic;
	[Always = UnorderedAccess] Texture VoxelAlbedoDynamic;
	[Always = UnorderedAccess] Texture VoxelNormalDynamic;
}

[Compute]
PassNode Lighting
{
	[Always = Read] Texture VSM_Atlas;
	[Always = Read] Texture VSM_PageTable;
	[Always = Read] StructuredBuffer<Camera> VSM_PageCameras;
	[Always = UnorderedAccess] Texture3D VoxelLighted;
	[Always = Read] Texture3D VoxelAlbedo;
	[Always = Read] Texture3D VoxelNormal;
	[Always = Read] TextureCube sky_cubemap_filtered;
	[Always = Read] Texture VoxelAlbedoStatic;
	[Always = Read] Texture VoxelNormalStatic;
	[Always = Read] Texture VoxelAlbedoDynamic;
	[Always = Read] Texture VoxelNormalDynamic;
}

[Compute]
PassNode Mipmapping
{
	[Always = UnorderedAccess] Texture3D VoxelLighted;
}

# Voxel-cone-traced indirect-GI signal (MyRaygenShader, raytracing.hlsl) --
# an alternative to IndirectRTX's raw RTX reference, selectable via
# g_indirect_source as NRD REBLUR_DIFFUSE's input (see
# [[project-nrd-integration]]). Not [Static]: depends on VoxelLighted, so
# only runs when VoxelGI's own volume is up to date.
[Compute]
PassNode VoxelScreen
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;
	[Always = Read] Texture3D VoxelLighted;
	[Always = Read] Texture BlueNoise;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture VoxelIndirectNoiseRaw;
}

# Voxel-cone-traced reflection signal (MyRaygenShaderReflection,
# raytracing.hlsl) -- an alternative to ReflectionRTX's raw RTX reference,
# selectable via g_reflection_source as NRD REBLUR_SPECULAR's input (see
# [[project-nrd-integration]]). Not [Static], same reasoning as VoxelScreen.
[Compute]
PassNode ScreenReflection
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment.
	[Always = Read] Texture GBuffer_Albedo;
	[Always = Read] Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	[Always = Read] Texture GBuffer_Specular;
	[Always = Read] Texture GBuffer_Speed;
	[Always = None] Texture GBuffer_DepthMips;
	Texture GBuffer_Quality;
	Texture GBuffer_DepthPrev;
	[Always = Read] Texture3D VoxelLighted;
	[Always = Read] Texture BlueNoise;

	[Always = UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R16G16B16A16_FLOAT] Texture VoxelReflectionNoiseRaw;
}
