// Test.HAL.SIG SIGCopyTexture: full-screen triangle that loads each texel from the CopyTexture slot's
// srcTex (Slots::Utility::CopyTexture), through the real generated binding header.
#include "../autogen/CopyTexture.h"

float4 VS(uint vid : SV_VertexID) : SV_Position
{
    float2 uv = float2((vid & 1) * 2.0, (vid >> 1) * 2.0);
    return float4(uv * 2.0 - 1.0, 0.0, 1.0);
}
float4 PS(float4 pos : SV_Position) : SV_Target
{
    return GetCopyTexture().GetSrcTex().Load(int3((int)pos.x, (int)pos.y, 0));
}
