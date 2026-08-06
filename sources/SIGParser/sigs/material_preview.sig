# Per-node preview capture for the material graph editor.
#
# A material's generated body is compiled once into this compute PSO
# instead of the production pixel PSO. The codegen (MaterialContext,
# see Materials/Values.cpp) injects one extra write per graph node right
# after that node's value is computed, so a single dispatch fills every
# node's preview slice at once instead of needing one PSO per node.
#
# Deliberately NOT sharing MaterialInfo/DefaultLayout::MaterialData: this
# keeps the preview path fully decoupled from the production material
# root signature used by every object in the real scene.
[Bind = DefaultLayout::Instance0]
struct MaterialPreviewInfo
{
	[dynamic] MaterialCB data;
	Texture2D<float4> textures[];
	RWTexture2DArray<float4> results;
}

# [Template]: real per-material source is always built manually (see
# MaterialPreviewSession::rebuild_pso) -- skip the unconditional startup
# build the .sig system otherwise gives every PSO, so material_preview.hlsl's
# placeholder MaterialCB never actually needs to compile into anything real.
[Template]
ComputePSO MaterialPreview
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = material_preview;
}

# 3D node preview: draws the real material_tester mesh (a direct, non-indirect
# dispatch_mesh -- see MaterialPreviewSession::dispatch) instead of dispatching
# over a flat/analytic-sphere quad. Reuses the same generic mesh/amplification
# shaders GBufferDraw/DrawAxis use (mesh_shader.hlsl) -- no material-specific
# geometry work needed, only the pixel shader differs. No render target: the
# pixel shader writes straight into MaterialPreviewInfo's results UAV, same
# UAV-only idea as Voxelization above -- with [earlydepthstencil] on
# PS_PREVIEW (see material_preview_3d.hlsl), since a UAV write isn't
# naturally gated by the depth test the way a normal render-target write is;
# without it, occluded fragments still executed and still wrote, which was
# the actual cause of the holes/flicker (an earlier cull=Front vs cull=Back
# comparison, made before that fix, was confounded by it and pointed the
# wrong way -- cull=Back is the standard/correct choice here).
[Template]
GraphicsPSO MaterialPreview3D
{
	root = DefaultLayout;

	[EntryPoint = VS]
	mesh = mesh_shader;

	[EntryPoint = AS]
	amplification = mesh_shader;

	[EntryPoint = PS_PREVIEW]
	pixel = material_preview_3d_stub;

	ds = D32_FLOAT;
	cull = Back;
	depth_func = GREATER;
}
