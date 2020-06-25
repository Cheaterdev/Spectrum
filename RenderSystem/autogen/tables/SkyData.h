#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SkyData
	{
		struct CB
		{
			float3 sunDir;
		} &cb;
		struct SRV
		{
			Render::Handle depthBuffer;
			Render::Handle transmittance;
			Render::Handle inscatter;
			Render::Handle irradiance;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		float3& GetSunDir() { return cb.sunDir; }
		Render::Handle& GetDepthBuffer() { return srv.depthBuffer; }
		Render::Handle& GetTransmittance() { return srv.transmittance; }
		Render::Handle& GetInscatter() { return srv.inscatter; }
		Render::Handle& GetIrradiance() { return srv.irradiance; }
		SkyData(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
