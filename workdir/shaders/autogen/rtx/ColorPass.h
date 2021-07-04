void ColorPass(RaytracingAccelerationStructure scene, RayDesc ray, RAY_FLAG flag, inout RayPayload payload) 
{
	TraceRay(scene, flag, ~0, 1, 0, 1, ray, payload); 
};
