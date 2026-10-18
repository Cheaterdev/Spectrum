// Test.HAL.Rendering RenderGeometryShader: VS emits one point at the origin; GS expands it into a
// full-screen orange triangle. Verifies the GS stage is invoked and produces visible output.
struct GSIn  { float4 pos : SV_Position; };
struct GSOut { float4 pos : SV_Position; float4 col : COLOR0; };

// VS: emit a single point — position is irrelevant, GS ignores it.
GSIn VS() { GSIn o; o.pos = float4(0,0,0,1); return o; }

// GS: one point in → one triangle out covering most of the screen.
[maxvertexcount(3)]
void GS(point GSIn input[1], inout TriangleStream<GSOut> stream)
{
    float4 orange = float4(1.0, 0.5, 0.0, 1.0);
    GSOut v;
    v.col = orange;
    v.pos = float4(-0.9, -0.9, 0, 1); stream.Append(v);
    v.pos = float4( 0.0,  0.9, 0, 1); stream.Append(v);
    v.pos = float4( 0.9, -0.9, 0, 1); stream.Append(v);
}

float4 PS(GSOut i) : SV_Target { return i.col; }
