
[Bind = FrameLayout::SceneData]
struct SceneData
{
	StructuredBuffer<node_data> nodes;
	StructuredBuffer<MeshCommandData> meshes;
	StructuredBuffer<MaterialCommandData> materials;

	StructuredBuffer<RaytraceInstanceInfo> raytraceInstanceInfo;

}



[Bind = DefaultLayout::Instance2]
[RenderTarget]
struct GBuffer
{
	RenderTarget<float4> albedo;
	RenderTarget<float4> normals;
	RenderTarget<float4> specular;
	RenderTarget<float2> motion;

	DepthStencil<float> depth;
}


[Base]
GraphicsPSO GBufferDraw
{
	root = DefaultLayout;
	
	[EntryPoint = VS]
	mesh  = mesh_shader;

	[EntryPoint = AS]
	amplification  = mesh_shader;

	rtv = { R8G8B8A8_UNORM, R8G8B8A8_UNORM, R8G8B8A8_UNORM, R16G16_FLOAT };

	ds = D32_FLOAT;
}



[Base]
GraphicsPSO DepthDraw
{
	root = DefaultLayout;

	[Erase]
	pixel = null;

	[EntryPoint = VS]
	mesh  = mesh_shader;

	[EntryPoint = AS]
	amplification  = mesh_shader;

	ds = D32_FLOAT;

	cull = Front;
}




[Base]
GraphicsPSO Voxelization
{
	root = DefaultLayout;
	
	[EntryPoint = VS]
	mesh  = mesh_shader_voxel;

	[EntryPoint = AS]
	amplification  = mesh_shader_voxel;

	[rename = VOXEL_DYNAMIC]
	[PS]
	define Dynamic;


	enable_depth = false;
	enable_stencil = false;
	cull = None;	
	conservative = true;
}