#pragma once


struct [raypayload]ShadowPayload
{	
	
	bool hit:read(anyhit,closesthit,miss,caller) : write(anyhit,closesthit,miss,caller);
	
};