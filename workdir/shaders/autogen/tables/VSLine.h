#pragma once
struct VSLine_cb
{
	float2 pos; // float2
	float4 color; // float4
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
