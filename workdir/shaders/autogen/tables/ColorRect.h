#pragma once
struct ColorRect_cb
{
	float4 color;
};
struct ColorRect
{
	ColorRect_cb cb;
	float4 GetColor() { return cb.color; }
};
 const ColorRect CreateColorRect(ColorRect_cb cb)
{
	const ColorRect result = {cb
	};
	return result;
}
