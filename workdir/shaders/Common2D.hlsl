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



#endif