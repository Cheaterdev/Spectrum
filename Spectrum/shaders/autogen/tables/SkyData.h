#pragma once
struct SkyData_cb
{
	float3 sunDir;
};
struct SkyData_srv
{
	Texture2D depthBuffer;
	Texture2D transmittance;
	Texture3D inscatter;
	Texture2D irradiance;
};
struct SkyData
{
	SkyData_cb cb;
	SkyData_srv srv;
	float3 GetSunDir() { return cb.sunDir; }
	Texture2D GetDepthBuffer() { return srv.depthBuffer; }
	Texture2D GetTransmittance() { return srv.transmittance; }
	Texture3D GetInscatter() { return srv.inscatter; }
	Texture2D GetIrradiance() { return srv.irradiance; }
};
 const SkyData CreateSkyData(SkyData_cb cb,SkyData_srv srv)
{
	const SkyData result = {cb,srv
	};
	return result;
}
