#pragma once
struct FontRenderingConstants_cb
{
	float4x4 TransformMatrix; // float4x4
	float4 ClipRect; // float4
};
struct FontRenderingConstants
{
	FontRenderingConstants_cb cb;
	float4x4 GetTransformMatrix() { return cb.TransformMatrix; }
	float4 GetClipRect() { return cb.ClipRect; }

};
 const FontRenderingConstants CreateFontRenderingConstants(FontRenderingConstants_cb cb)
{
	const FontRenderingConstants result = {cb
	};
	return result;
}
