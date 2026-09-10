
[Bind = FrameLayout::SceneData]
struct SceneData
{
	StructuredBuffer<node_data> nodes;
	StructuredBuffer<MeshCommandData> meshes;
	StructuredBuffer<MaterialCommandData> materials;

	StructuredBuffer<RaytraceInstanceInfo> raytraceInstanceInfo;

	RaytracingAccelerationStructure scene;
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
	mesh  = gbuffer/mesh_shader;

	[EntryPoint = AS]
	amplification  = gbuffer/mesh_shader;

	# Per-meshlet Hi-Z occlusion in the AS. Off for the occlusion culler's
	# stage 1 (it would test against last frame's pyramid with this frame's
	# camera, and a wrongly culled meshlet is never re-tested), on for stage 2.
	[rename = HIZ_OCCLUSION]
	[AS, nullable]
	define HiZOcclusion;

	rtv = { R8G8B8A8_UNORM, R8G8B8A8_UNORM, R8G8B8A8_UNORM, R16G16_FLOAT };

	ds = D32_FLOAT;
	cull = None;
}



[Base]
GraphicsPSO DepthDraw
{
	root = DefaultLayout;

	[Erase]
	pixel = null;

	[EntryPoint = VS]
	mesh  = gbuffer/mesh_shader;

	[EntryPoint = AS]
	amplification  = gbuffer/mesh_shader;

	# See GBufferDraw.
	[rename = HIZ_OCCLUSION]
	[AS, nullable]
	define HiZOcclusion;

	ds = D32_FLOAT;

	cull = Front;
}




[Base]
GraphicsPSO Voxelization
{
	root = DefaultLayout;
	
	[EntryPoint = VS]
	mesh  = voxelgi/mesh_shader_voxel;

	[EntryPoint = AS]
	amplification  = voxelgi/mesh_shader_voxel;

	[rename = VOXEL_DYNAMIC]
	[PS]
	define Dynamic;


	enable_depth = false;
	enable_stencil = false;
	cull = None;	
	conservative = true;
}



[Static]
PassNode PreScene
{
	[Always = UnorderedAccess] [Size = 1] StructuredBuffer<uint> scene;
}



[Static]
[Required]
# TriState: this pass exists purely to keep swapchain graph-tracked -- it
# never actually renders anything itself (some other UI/overlay pass owns
# the real profiler drawing), so setup() always returns IgnoreRender, never
# NeedsRender.
[TriState]
PassNode Profiler
{
	[Always = Required | RenderTarget] Texture swapchain;
}

[Static]
[Compute]
# Dead: PassDefault<Passes::CopyPrev>::setup() (PassDefaults.cpp) always
# returns false -- kept disabled until dropped from the .sig entirely (see
# its own comment there). GBuffer_SpecularPrev dropped already: its history
# was unused (denoiser roughness-history disabled) and removed along with
# this pass's real body, per PassDefaults.cpp's own comment.
PassNode CopyPrev
{
	[Write] Texture GBuffer_DepthPrev;
	[Write] Texture GBuffer_NormalsPrev;
}

[Static]
PassNode Scene
{
	# Flat fields, not the (removed) GBuffer PassView -- see pssm.sig's own
	# comment. GBuffer_SpecularPrev dropped (see CopyPrev's own comment,
	# above) -- Scene never actually created it even before flattening.
	#
	# Auto-created (ViewportContext-driven, same mechanism as GBuffer_HiZ
	# elsewhere). GBuffer_Normals/GBuffer_DepthMips are each a [PrevFor=X]
	# field's linked "current" below -- create_always() (pass.jinja) emits
	# their builder.create_prev() call immediately after this create(), same
	# position a hand-written call would need, so no manual setup() code is
	# needed for GBuffer at all any more. link_history() itself runs even
	# earlier, auto-generated into link_history_always() (before setup_func)
	# from each *Prev field's own [PrevFor=X] -- see its own comment
	# (pass.jinja) for why that ordering works for an auto-created X too.
	[Always = RenderTarget] [Size = ViewportContext::frame_size] [Format = R8G8B8A8_UNORM] Texture GBuffer_Albedo;
	[Always = RenderTarget | UnorderedAccess] [Size = ViewportContext::frame_size] [Format = R8G8B8A8_UNORM] Texture GBuffer_Normals;
	[Always = DepthStencil] [Size = ViewportContext::frame_size] [Format = R32_TYPELESS] Texture GBuffer_Depth;
	[Always = RenderTarget] [Size = ViewportContext::frame_size] [Format = R8G8B8A8_UNORM] Texture GBuffer_Specular;
	[Always = RenderTarget] [Size = ViewportContext::frame_size] [Format = R16G16_FLOAT] Texture GBuffer_Speed;
	[Always = UnorderedAccess | RenderTarget] [Size = ViewportContext::frame_size] [Format = R32_TYPELESS] Texture GBuffer_DepthMips;
	[Always = DepthStencil] [Size = ViewportContext::frame_size] [Format = D24_UNORM_S8_UINT] Texture GBuffer_Quality;
	[PrevFor = GBuffer_Normals] [Write] Texture GBuffer_NormalsPrev;
	[PrevFor = GBuffer_DepthMips] [Always = Read] Texture GBuffer_DepthPrev;
	# Static: pass 1 of the GPU occlusion culler tests boxes against LAST
	# frame's HiZ, so the contents must survive across frames (no aliasing).
	# MipCount=0: full auto mip chain (a Hi-Z pyramid), matching the original
	# manual create()'s omitted 4th Desc field.
	[Always = DepthStencil | Static] [Size = `builder.graph->get_context<Table::ViewportContext>().frame_size / 8`] [Format = R32_TYPELESS] [MipCount = 0]
	Texture GBuffer_HiZ;
	[Always = UnorderedAccess] [Size = `builder.graph->get_context<Table::ViewportContext>().frame_size / 8`] [Format = R32_FLOAT] [MipCount = 0]
	Texture GBuffer_HiZ_UAV;
	[Always = Read] StructuredBuffer<uint> scene;
}
