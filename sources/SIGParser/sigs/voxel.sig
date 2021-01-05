struct VoxelTilingParams
{
	uint4 voxels_per_tile;
	StructuredBuffer<int3> tiles;

	%{
		
uint3 get_voxel_pos(uint3 dispatchID)
{
	uint tile_index = dispatchID.x / cb.voxels_per_tile.x;
	uint3 tile_pos = srv.tiles[tile_index] * cb.voxels_per_tile.xyz;

	uint3 tile_local_pos = dispatchID - int3(tile_index * cb.voxels_per_tile.x, 0, 0);
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


[Bind = DefaultLayout::Instance2]
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
}

[Bind = DefaultLayout::Instance2]
struct VoxelBlur
{
	Texture2D<float4> tex_color;
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

GraphicsPSO VoxelReflectionLow
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxel_screen;

	[EntryPoint = PS_low]
	pixel = voxel_screen_reflection;

	enable_depth = false;
}

GraphicsPSO VoxelIndirectFilter
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxel_screen;

	[EntryPoint = PS]
	pixel = voxel_screen_blur;

	enable_depth = false;

	
	rtv = { DXGI_FORMAT_R11G11B10_FLOAT};
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
	stencil_pass_op = KEEP;

	rtv = {DXGI_FORMAT_R16G16B16A16_FLOAT};
	ds = DXGI_FORMAT_D24_UNORM_S8_UINT;

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
	stencil_pass_op = KEEP;

	rtv = {DXGI_FORMAT_R16G16B16A16_FLOAT};
	ds = DXGI_FORMAT_D24_UNORM_S8_UINT;

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
	stencil_pass_op = KEEP;

	blend = { Additive , None};
	rtv = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R11G11B10_FLOAT};
	ds = DXGI_FORMAT_D24_UNORM_S8_UINT;

	stencil_read_mask = 1;
	stencil_write_mask = 1;
}


GraphicsPSO VoxelIndirectLow
{
	root = DefaultLayout;

	[EntryPoint = VS]
	vertex = voxel_screen;

	[EntryPoint = PS_Low]
	pixel = voxel_screen;

	enable_depth = false;
	
	rtv = { DXGI_FORMAT_R11G11B10_FLOAT};
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
	stencil_pass_op = KEEP;

	blend = { Additive , None};
	rtv = { DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R11G11B10_FLOAT};
	ds = DXGI_FORMAT_D24_UNORM_S8_UINT;

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

	rtv = {DXGI_FORMAT_R16G16B16A16_FLOAT};
	enable_depth = false;
}