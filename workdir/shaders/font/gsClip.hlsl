#include "../autogen/FontRendering.h"
#include "../autogen/FontRenderingConstants.h"


static const Buffer<float4> tex0 = GetFontRendering().GetPositions();
static const float4x4  TransformMatrix = GetFontRenderingConstants().GetTransformMatrix();
static const float4  ClipRect = GetFontRenderingConstants().GetClipRect();

struct GSIn
{
    float4 Position : SV_Position;
    float4 GlyphColor : COLOR;
    float index : TEXCOORD;
};

struct GSOut
{
    float4 Position : SV_Position;
    float4 GlyphColor : COLOR;
    float2 TexCoord : TEXCOORD;
    float4 ClipDistance : SV_ClipDistance;
};

[maxvertexcount(4)]
void GS(point GSIn Input[1], inout TriangleStream<GSOut> TriStream)
{
    const float2 basePosition = Input[0].Position.xy;
    const uint glyphIndex = uint(Input[0].index);
    float4 texCoords = tex0.Load(uint2(glyphIndex * 2, 0));
    float4 offsets = tex0.Load(uint2(glyphIndex * 2 + 1, 0));
    GSOut Output;
    Output.GlyphColor = Input[0].GlyphColor;
    float4 positions = basePosition.xyxy + offsets;
    Output.Position = mul(TransformMatrix, float4(positions.xy, 0.0f, 1.0f));
    Output.TexCoord = texCoords.xy;
    Output.ClipDistance = ClipRect + float4(positions.xy, -positions.xy);
    TriStream.Append(Output);
    Output.Position = mul(TransformMatrix, float4(positions.zy, 0.0f, 1.0f));
    Output.TexCoord = texCoords.zy;
    Output.ClipDistance = ClipRect + float4(positions.zy, -positions.zy);
    TriStream.Append(Output);
    Output.Position = mul(TransformMatrix, float4(positions.xw, 0.0f, 1.0f));
    Output.TexCoord = texCoords.xw;
    Output.ClipDistance = ClipRect + float4(positions.xw, -positions.xw);
    TriStream.Append(Output);
    Output.Position = mul(TransformMatrix, float4(positions.zw, 0.0f, 1.0f));
    Output.TexCoord = texCoords.zw;
    Output.ClipDistance = ClipRect + float4(positions.zw, -positions.zw);
    TriStream.Append(Output);
    TriStream.RestartStrip();
}
;