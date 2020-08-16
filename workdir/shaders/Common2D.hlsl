#ifndef COMMON_2D_HLSL
#define COMMON_2D_HLSL

#include "Common.hlsl"

struct vertex_input
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 tc : TEXCOORD;
	float4 tangent : TANGENT;
};

struct vertex_output
{
	float4 pos : SV_POSITION;
};


vertex_output transform(camera_info camera, matrix mat, vertex_input i)
{
	vertex_output o;
	float4 tpos = mul(mat, float4(i.pos.xyz, 1));
	o.pos = mul(camera.view_proj, tpos);

	return o;
}


#endif