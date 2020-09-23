#pragma once
struct VSLine_cb
{
	float2 pos;
	float4 color;
};
struct VSLine
{
	VSLine_cb cb;
	float2 GetPos() { return cb.pos; }
	float4 GetColor() { return cb.color; }

};
 const VSLine CreateVSLine(VSLine_cb cb)
{
	const VSLine result = {cb
	};
	return result;
}
