#pragma once
struct SkyData
{
	float3 sunDir; // float3
	uint depthBuffer; // Texture2D<float>
	uint transmittance; // Texture2D<float4>
	uint inscatter; // Texture3D<float4>
	uint irradiance; // Texture2D<float4>
	float3 GetSunDir() { return sunDir; }
	Texture2D<float> GetDepthBuffer() { return ResourceDescriptorHeap[depthBuffer]; }
	Texture2D<float4> GetTransmittance() { return ResourceDescriptorHeap[transmittance]; }
	Texture3D<float4> GetInscatter() { return ResourceDescriptorHeap[inscatter]; }
	Texture2D<float4> GetIrradiance() { return ResourceDescriptorHeap[irradiance]; }
};
