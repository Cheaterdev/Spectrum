#pragma once
struct ColorRect_cb
{
	float4 pos[2]; // float4
	float4 color; // float4
};
struct ColorRect
{
	ColorRect_cb cb;
	float4 GetPos(int i) { return cb.pos[i]; }
	float4 GetColor() { return cb.color; }

};
 const ColorRect CreateColorRect(ColorRect_cb cb)
{
	const ColorRect result = {cb
	};
	return result;
}
