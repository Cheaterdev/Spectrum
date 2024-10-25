#pragma once


struct [raypayload] RayCone
{	
	
	float width : read(anyhit,closesthit,miss,caller) : write(anyhit,closesthit,miss,caller);
	float angle : read(anyhit,closesthit,miss,caller) : write(anyhit,closesthit,miss,caller);
	
	RayCone propagate(float surfaceSpreadAngle = 0, float hitT = 0)
	{
		RayCone result;
		result.width = width + angle* hitT;
		result.angle = angle + surfaceSpreadAngle;
		
		return result;
	}

	
};