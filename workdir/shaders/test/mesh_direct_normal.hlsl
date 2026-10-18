// Test.HAL.Rendering RenderMeshDirect: the engine's mesh/amplification stages (gbuffer/mesh_shader.hlsl)
// with a trivial normal-visualization pixel shader.
#include "../gbuffer/mesh_shader.hlsl"
float4 PS(vertex_output i) : SV_Target { return float4(i.normal * 0.5 + 0.5, 1); }
