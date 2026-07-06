#ifndef RAYTRACING_DEBUG_HLSL
#define RAYTRACING_DEBUG_HLSL

// Debug-only raygen for the RTX scene view. Kept self-contained (no include of
// raytracing.hlsl) so it doesn't drag in the voxel bindings and their slots.
#include "autogen/FrameInfo.h"
#include "autogen/Raytracing.h"
#include "autogen/tables/RayPayload.h"
#include "autogen/rtx/ColorPass.h"
#include "autogen/ColorRTXOutput.h"

// Generate a world-space ray for a camera pixel from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, in Camera camera, out float3 origin, out float3 direction)
{
	float2 xy = index + 0.5f; // center in the middle of the pixel.
	float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

	// Invert Y for DirectX-style coordinates.
	screenPos.y = -screenPos.y;

	// Unproject the pixel coordinate into a ray.
	float4 world = mul(camera.GetInvViewProj(), float4(screenPos, 0, 1));

	world.xyz /= world.w;
	origin = camera.GetPosition().xyz;
	direction = normalize(world.xyz - origin);
}

// Primary-ray color pass: one ray per pixel from the camera, color from the
// material closest-hit shader, written into the dedicated ColorRTXOutput UAV.
[shader("raygeneration")]
void ColorRTXRaygenShader()
{
	uint2 itc = DispatchRaysIndex().xy;

	const Raytracing     raytracing = CreateRaytracing();
	const FrameInfo      frame      = CreateFrameInfo();
	const ColorRTXOutput out_data   = CreateColorRTXOutput();

	float3 origin, direction;
	GenerateCameraRay(itc, frame.GetCamera(), origin, direction);

	[raypayload] RayPayload payload;
	payload.init();

	RayDesc ray;
	ray.Origin    = origin;
	ray.Direction = direction;
	ray.TMin      = 0.01;
	ray.TMax      = 10000.0;
	ColorPass(raytracing.GetScene(), ray, RAY_FLAG_NONE, payload);

	out_data.GetOutput()[itc] = payload.color;
}

#endif // RAYTRACING_DEBUG_HLSL
