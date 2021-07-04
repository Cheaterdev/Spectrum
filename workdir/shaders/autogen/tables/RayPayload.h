#pragma once
#include "RayCone.h"
struct RayPayload
{
	float4 color;
	float3 dir;
	uint recursion;
	float dist;
	RayCone cone;


	RayPayload propagate(float surfaceSpreadAngle = 0, float hitT = 0)
	{
		RayPayload result;

		result.color = 0;
		result.dist = 0;
		result.recursion = recursion + 1;

		result.cone = cone.propagate(surfaceSpreadAngle, hitT);
		
		return result;
	}


	void init()
	{
		color = 0;
		recursion = 0;
		dist = 0;
		cone.angle = 0;
		cone.width = 0; 
	}

	
};
