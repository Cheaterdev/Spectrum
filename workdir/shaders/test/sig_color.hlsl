// Test.HAL.SIG SIGColor: full-screen triangle that outputs the Color slot (Slots::Editor::Color), through the
// real generated binding header.
#include "../autogen/Color.h"

float4 VS(uint vid : SV_VertexID) : SV_Position
{
    float2 uv = float2((vid & 1) * 2.0, (vid >> 1) * 2.0);
    return float4(uv * 2.0 - 1.0, 0.0, 1.0);
}
float4 PS() : SV_Target
{
    return GetColor().GetColor();
}
