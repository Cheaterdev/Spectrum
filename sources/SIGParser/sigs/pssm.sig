

# Fused half-res GBuffer downsample + generic 8x8-tile classification. One
# 8x8 thread group per screen tile: loads the tile's full-res depth+normal
# into LDS, picks the closest (max depth, reversed-Z) real sample per 2x2
# block as the half-res output -- same rule GBufferDownsampleRT's old PS
# used, never an average, so nothing downstream infers geometry that isn't
# really there -- then classifies the whole tile Hi/Low from the spread of
# those 16 representatives, and flags individual full-res pixels whose own
# depth diverges too far from their block's representative in tile_mask.
# Generic infra: not tied to GI/reflections/shadows specifically, any pass
# needing "does this tile need full-res work" reads TileClassifyHi/Low/Mask/
# Tiles by name. tile_hi/tile_low are compacted lists (for a future indirect-
# dispatch consumer); tile_flags is the same verdict as a point-queryable
# tile-resolution texture (1 = Hi), for a shader that just wants to test its
# own tile without a full indirect-dispatch setup -- IndirectRTX's raygen is
# the first consumer of this form.
[Bind = DefaultLayout::Instance0]
struct TileClassifyData
{
	GBuffer gbuffer;

	RWTexture2D<float> half_depth;
	RWTexture2D<float4> half_normals;

	AppendStructuredBuffer<uint2> tile_hi;
	AppendStructuredBuffer<uint2> tile_low;

	RWTexture2D<uint> tile_mask;
	RWTexture2D<uint> tile_flags;
}



[Bind = DefaultLayout::Instance2]
struct GBufferQuality
{
	Texture2D<float4> ref;
}

[Bind = DefaultLayout::Instance0]
struct PSSMConstants
{
	int level;
	float time;
}

[Bind = DefaultLayout::Instance1]
struct PSSMData
{	
	Texture2DArray<float> light_buffer;
	StructuredBuffer<Camera> light_cameras;
}

[Bind = DefaultLayout::Instance1]
struct PSSMDataGlobal
{	
	Texture2D<float> light_buffer;
	StructuredBuffer<Camera> light_camera;
}


[Bind = DefaultLayout::Instance2]
struct PSSMLighting
{
	GBuffer gbuffer;
	Texture2D<float> light_mask;
	RWTexture2D<float4> result;
}


GraphicsPSO PSSMMask
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = shadows/pssm/pssm;

	[EntryPoint = PS]
	pixel = shadows/pssm/pssm;

	rtv = { R8_UNORM };
}

GraphicsPSO PSSMApply
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = shadows/pssm/pssm;

	[EntryPoint = PS_RESULT]
	pixel = shadows/pssm/pssm;

	rtv = { R16G16B16A16_FLOAT };
}

ComputePSO PSSMApplyCompute
{
	root = DefaultLayout;

	[EntryPoint = CS_RESULT]
	compute = shadows/pssm/pssm;
}


ComputePSO GBufferDownsample
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = postprocess/downsample;
}


PassView GBuffer
{
	Texture GBuffer_Albedo;
	Texture GBuffer_Normals;
	Texture GBuffer_Depth;
	Texture GBuffer_Specular;

	Texture GBuffer_Speed;
	Texture GBuffer_DepthMips;


	Texture GBuffer_Quality;
	Texture GBuffer_TempColor;

	Texture GBuffer_NormalsPrev;
	Texture GBuffer_SpecularPrev;
	Texture GBuffer_DepthPrev;

	Texture GBuffer_HiZ;
	Texture GBuffer_HiZ_UAV;
}

[Multiple = 6]
PassNode PSSM_Cascade
{
	[Write] Texture PSSM_Depths;
	[Write] StructuredBuffer<Camera> PSSM_Cameras;
}

PassNode PSSM_GenerateMask
{
	Texture PSSM_Depths;
	StructuredBuffer<Camera> PSSM_Cameras;

	GBuffer gbuffer;
	[Write] Texture LightMask;
}

[Compute]
PassNode PSSM_Combine
{
	StructuredBuffer<Camera> PSSM_Cameras;
	GBuffer gbuffer;
	Texture LightMask;
	Texture ShadowMask;
	[Write] Texture ResultTexture;
}

PassNode PSSM_Global
{
	[Write] Texture global_depth;
	[Write] StructuredBuffer<Camera> global_camera;
}