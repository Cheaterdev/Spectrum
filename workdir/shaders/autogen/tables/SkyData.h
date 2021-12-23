#pragma once
struct SkyData_cb
{
	float3 sunDir; // float3
};
struct SkyData_srv
{
	uint depthBuffer; // Texture2D<float>
	uint transmittance; // Texture2D<float4>
	uint inscatter; // Texture3D<float4>
	uint irradiance; // Texture2D<float4>
};
struct SkyData
{
	SkyData_cb cb;
	SkyData_srv srv;
	Texture2D<float> GetDepthBuffer() { return ResourceDescriptorHeap[srv.depthBuffer]; }
	Texture2D<float4> GetTransmittance() { return ResourceDescriptorHeap[srv.transmittance]; }
	Texture3D<float4> GetInscatter() { return ResourceDescriptorHeap[srv.inscatter]; }
	Texture2D<float4> GetIrradiance() { return ResourceDescriptorHeap[srv.irradiance]; }
	float3 GetSunDir() { return cb.sunDir; }

};
 const SkyData CreateSkyData(SkyData_cb cb,SkyData_srv srv)
{
	const SkyData result = {cb,srv
	};
	return result;
}
