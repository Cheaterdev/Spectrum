// Test.HAL.Rendering RenderInstancing: 9 instances of a small triangle arranged in a 3×3 grid.
// SV_InstanceID drives position (column/row offset) and color.
static const float2 kLocalPos[3] = {
    float2(-0.12, -0.10),
    float2( 0.12, -0.10),
    float2( 0.00,  0.12),
};
static const float4 kColor[9] = {
    float4(1,0,0,1),   float4(0,1,0,1),   float4(0,0,1,1),
    float4(1,1,0,1),   float4(1,0,1,1),   float4(0,1,1,1),
    float4(1,0.5,0,1), float4(0,0.5,1,1), float4(0.5,1,0,1),
};
struct VSOut { float4 pos : SV_Position; float4 col : COLOR0; };
VSOut VS(uint vid : SV_VertexID, uint iid : SV_InstanceID)
{
    uint col = iid % 3;
    uint row = iid / 3;
    float2 offset = float2((float(col) - 1.0) * 0.62,
                           (1.0 - float(row)) * 0.62);
    VSOut o;
    o.pos = float4(kLocalPos[vid] + offset, 0.0, 1.0);
    o.col = kColor[iid];
    return o;
}
float4 PS(VSOut i) : SV_Target { return i.col; }
