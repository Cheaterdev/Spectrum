#ifndef DRAW_DATA_HLSL
#define DRAW_DATA_HLSL

// Per-draw data of mesh draws, the same way on D3D12 and Vulkan.
//
// An indirect draw is a DrawMeshCommand { draw_id, args } (meshrender.prism):
// commands carry no slots, because Vulkan cannot set per-draw constants from a
// command buffer. The draw id names the mesh (SceneData.meshes, low 24 bits;
// the high 8 bits are free for the pass -- VSM: the clipmap level), and the
// MeshInfo / MeshInstanceInfo / MaterialInfo tables are looked up from it.
//
// A direct draw (IndirectDraw slot == DRAW_ID_NONE, the HAL's default on
// set_pipeline) keeps reading those tables' own slots.
//
// Include this INSTEAD of MeshInfo.h / MeshInstanceInfo.h / SceneData.h (and
// MaterialInfo.h): generated slot headers error out when included twice.
// MaterialInfo needs the material's own MaterialCB, so only material shaders
// get it, by defining DRAW_DATA_MATERIAL first.

#include "../autogen/SceneData.h"
#include "../autogen/MeshInfo.h"
#include "../autogen/MeshInstanceInfo.h"
#ifdef DRAW_DATA_MATERIAL
#include "../autogen/MaterialInfo.h"
#endif

// On D3D12 the IndirectDraw slot holds a raw draw id, not a table: never build
// its global accessor.
#ifndef NO_GLOBAL
#define NO_GLOBAL
#define DRAW_DATA_UNDEF_NO_GLOBAL
#endif
#include "../autogen/IndirectDraw.h"
#ifdef DRAW_DATA_UNDEF_NO_GLOBAL
#undef NO_GLOBAL
#undef DRAW_DATA_UNDEF_NO_GLOBAL
#endif

static const uint DRAW_ID_NONE      = 0xFFFFFFFF;   // HAL: DRAW_ID_NONE (SIG.ixx)
static const uint DRAW_ID_MESH_MASK = 0x00FFFFFF;

#ifdef __spirv__
// 4426 = DrawIndex, 4427 = DrawParameters (device feature shaderDrawParameters).
[[vk::ext_extension("SPV_KHR_shader_draw_parameters")]]
[[vk::ext_capability(4427)]]
[[vk::ext_builtin_input(4426)]]
static const uint gl_DrawIndex;

// 5271 = PerPrimitiveEXT, 5283 = MeshShadingEXT: a pixel-shader input written
// per primitive by the mesh shader.
#ifdef VERTEX_FRONT_END
// Behind a vertex shader (gbuffer/vertex_shader.hlsl) the same value is a flat per-vertex output.
#define PER_PRIMITIVE_INPUT nointerpolation
#else
#define PER_PRIMITIVE_INPUT [[vk::ext_extension("SPV_EXT_mesh_shader")]] [[vk::ext_capability(5283)]] [[vk::ext_decorate(5271)]]
#endif
#else
#define PER_PRIMITIVE_INPUT
#endif

// The draw id, in task/mesh shaders. D3D12: the IndirectDraw root constant,
// written per draw by the DrawMeshCommand signature. Vulkan: this draw's own
// command, found by DrawIndex (not available to pixel shaders -- those get the
// id per primitive from the mesh shader).
uint read_draw_id()
{
	const uint slot = pass_IndirectDraw.offset;
	if (slot == DRAW_ID_NONE)
		return DRAW_ID_NONE;
#ifdef __spirv__
	IndirectDraw draw = CreateIndirectDraw();
	return draw.GetCommands()[draw.GetBase() + gl_DrawIndex].draw_id;
#else
	return slot;
#endif
}

MeshCommandData draw_mesh(uint draw_id)
{
	return GetSceneData().GetMeshes()[draw_id & DRAW_ID_MESH_MASK];
}

ConstantBuffer<MeshInfo> draw_mesh_info(uint draw_id)
{
	const uint index = draw_id == DRAW_ID_NONE ? pass_MeshInfo.offset : draw_mesh(draw_id).mesh_cb;
	return ResourceDescriptorHeap[index];
}

ConstantBuffer<MeshInstanceInfo> draw_mesh_instance_info(uint draw_id)
{
	const uint index = draw_id == DRAW_ID_NONE ? pass_MeshInstanceInfo.offset : draw_mesh(draw_id).meshinstance_cb;
	return ResourceDescriptorHeap[index];
}

#ifdef DRAW_DATA_MATERIAL
ConstantBuffer<MaterialInfo> draw_material_info(uint draw_id)
{
	const uint index = draw_id == DRAW_ID_NONE
		? pass_MaterialInfo.offset
		: GetSceneData().GetMaterials()[draw_mesh(draw_id).material_id].material_cb;
	return ResourceDescriptorHeap[index];
}
#endif

#endif
