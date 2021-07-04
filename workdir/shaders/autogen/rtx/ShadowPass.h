void ShadowPass(RaytracingAccelerationStructure scene, RayDesc ray, RAY_FLAG flag, inout ShadowPayload payload) 
{
	TraceRay(scene, flag, ~0, 0, 0, 0, ray, payload); 
};
