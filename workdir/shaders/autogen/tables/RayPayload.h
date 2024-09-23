#pragma once

#include "RayCone.h"
struct [raypayload]RayPayload
{	
	
	float4 color:read(anyhit,closesthit,miss,caller) : write(anyhit,closesthit,miss,caller);
	float3 dir:read(anyhit,closesthit,miss,caller) : write(anyhit,closesthit,miss,caller);
	uint recursion:read(anyhit,closesthit,miss,caller) : write(anyhit,closesthit,miss,caller);
	float dist:read(anyhit,closesthit,miss,caller) : write(anyhit,closesthit,miss,caller);

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