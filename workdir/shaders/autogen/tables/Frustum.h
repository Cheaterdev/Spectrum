#pragma once
struct Frustum
{
	float4 planes[6]; // float4
	float4 GetPlanes(int i) { return planes[i]; }
};
