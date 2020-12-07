#pragma once
struct SkyData_cb
{
	float3 sunDir;
};
struct SkyData_srv
{
	Texture2D<float> depthBuffer;
	Texture2D<float4> transmittance;
	Texture3D<float4> inscatter;
	Texture2D<float4> irradiance;
};
struct SkyData
{
	SkyData_cb cb;
	SkyData_srv srv;
	float3 GetSunDir() { return cb.sunDir; }
	Texture2D<float> GetDepthBuffer() { return srv.depthBuffer; }
	Texture2D<float4> GetTransmittance() { return srv.transmittance; }
	Texture3D<float4> GetInscatter() { return srv.inscatter; }
	Texture2D<float4> GetIrradiance() { return srv.irradiance; }

};
 const SkyData CreateSkyData(SkyData_cb cb,SkyData_srv srv)
{
	const SkyData result = {cb,srv
	};
	return result;
}
