// Test.HAL.Rendering RenderCull_*: two triangles of opposite winding; the PSO's cull mode decides which show.
// Left triangle  — CCW in NDC (signed area > 0) = front face = GREEN
// Right triangle — CW  in NDC (signed area < 0) = back  face = RED
// With BackCull:  only GREEN visible.
// With FrontCull: only RED  visible.
// With NoCull:    both visible.
static const float2 kPos[6] = {
    float2(-0.7,-0.5), float2(-0.1,-0.5), float2(-0.4, 0.5),  // CCW = front face
    float2( 0.1,-0.5), float2( 0.4, 0.5), float2( 0.7,-0.5),  // CW  = back  face
};
static const float4 kColor[2] = {
    float4(0.0,1.0,0.0,1.0),  // GREEN = front face
    float4(1.0,0.0,0.0,1.0),  // RED   = back  face
};
struct VSOut { float4 pos : SV_Position; float4 col : COLOR0; };
VSOut VS(uint vid : SV_VertexID)
{
    VSOut o;
    o.pos = float4(kPos[vid], 0.0, 1.0);
    o.col = kColor[vid / 3];
    return o;
}
float4 PS(VSOut i) : SV_Target { return i.col; }
