#pragma once
struct Color_cb
{
	float4 color; // float4
};
struct Color
{
	Color_cb cb;
	float4 GetColor() { return cb.color; }

};
 const Color CreateColor(Color_cb cb)
{
	const Color result = {cb
	};
	return result;
}
