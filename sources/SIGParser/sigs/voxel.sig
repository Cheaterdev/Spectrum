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

	Texture2D<float4> prev_gi;

	Texture2D<float> prev_frames;
	Texture2D<float> prev_depth;
}

[Bind = DefaultLayout::Instance2]
struct VoxelOutput
{
	RWTexture2D<float4> noise;
	RWTexture2D<float> frames;

	RWTexture2D<float4> DirAndPdf;
	 Texture2D<float2> blueNoise;

}

[Bind = DefaultLayout::Instance2]
struct VoxelBlur
{
	Texture2D<float4> noisy_output;
	Texture2D<float4> hit_and_pdf;

	Texture2D<float4> prev_result;

	RWTexture2D<float4> screen_result;
	RWTexture2D<float4> gi_result;
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

    PSSMDataGlobal pssmGlobal;

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
	compute = voxel_lighting;

	[rename = SECOND_BOUNCE]
	[CS, nullable]
	define SecondBounce;
}


ComputePSO VoxelDownsample
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxel_mipmap;

	[rename = COUNT]
	[CS]
	define Count = {1, 2, 3};
}



ComputePSO VoxelCopy
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxel_copy;
}


ComputePSO VoxelZero
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxel_zero;
}

ComputePSO VoxelVisibility
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = voxel_visibility;
}

ComputePSO VoxelIndirectFilter
{
	root = DefaultLayout;

	[EntryPoint = PS]
	compute = voxel_screen_blur;

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
	vertex = voxel_screen;

	[EntryPoint = PS]
	pixel = voxel_screen_reflection;

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
	vertex = voxel_screen;

	[EntryPoint = PS_resize]
	pixel = voxel_screen_reflection;

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
	vertex = voxel_screen;

	[EntryPoint = PS]
	pixel = voxel_screen;

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
	compute = voxel_screen;
}



GraphicsPSO VoxelIndirectUpsample
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxel_screen;

	[EntryPoint = PS_Resize]
	pixel = voxel_screen;

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
	vertex = voxel_screen;

	[EntryPoint = Debug]
	pixel = voxel_screen_debug;

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
	vertex = DenoiserDownsample;

	[EntryPoint = PS]
	pixel = DenoiserDownsample;

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
	compute = DenoiserHistoryFix;
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
	compute = FrameClassification;
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
	compute = FrameClassificationInitDispatch;
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
	compute = reflection_combine;
}
