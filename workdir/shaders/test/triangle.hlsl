// Test.HAL.Rendering RenderTriangle: orange triangle, VS positions via SV_VertexID in clip space.
float4 VS(uint vertID : SV_VertexID) : SV_Position
{
    float2 pos[3];
    pos[0] = float2(-0.5, -0.5);
    pos[1] = float2( 0.0,  0.5);
    pos[2] = float2( 0.5, -0.5);
    return float4(pos[vertID], 0.0, 1.0);
}
float4 PS() : SV_Target
{
    return float4(1.0, 0.5, 0.0, 1.0);
}
