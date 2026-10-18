// The GBuffer draw for GPUs without mesh shaders: a plain vertex stage over an indexed draw, feeding the
// same material pixel shader as mesh_shader.hlsl does. Everything the mesh stage computes per vertex
// (transform, the draw's tables) comes from mesh_shader.hlsl itself, so the interpolants the pixel
// shader reads are identical by construction. VERTEX_FRONT_END drops that file's mesh stage and makes the
// draw id a flat vertex output (draw_data.hlsl: PER_PRIMITIVE_INPUT).
#define VERTEX_FRONT_END
#include "mesh_shader.hlsl"

// Same member order as the mesh shader's vertex_output plus its per-primitive draw id, so the pixel shader's
// inputs land on the same locations.
struct vertex_front_output
{
    vertex_output v;
    nointerpolation uint draw_id_out : DRAW_ID;
};

// Direct indexed draws: the draw's MeshInfo / MeshInstanceInfo / MaterialInfo come from their own slots
// (IndirectDraw == DRAW_ID_NONE), and the index buffer holds mesh-local vertex numbers.
vertex_front_output VSVertex(uint vertex_id : SV_VertexID)
{
    bind_draw(read_draw_id());

    node_data node = sceneData.GetNodes()[meshInfo.GetNode_offset()];

    vertex_front_output o;
    o.v = transform(node.GetNode_global_matrix(), node.GetNode_global_matrix_prev(), frameInfo.GetCamera(),
                    meshInstanceInfo.GetVertexes()[meshInfo.GetVertex_offset_local() + vertex_id]);
    o.draw_id_out = draw_id;
    return o;
}
