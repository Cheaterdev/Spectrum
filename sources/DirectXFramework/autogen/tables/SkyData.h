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
			Render::HLSL::Texture2D<float> depthBuffer;
			Render::HLSL::Texture2D<float4> transmittance;
			Render::HLSL::Texture3D<float4> inscatter;
			Render::HLSL::Texture2D<float4> irradiance;
		} &srv;
		float3& GetSunDir() { return cb.sunDir; }
		Render::HLSL::Texture2D<float>& GetDepthBuffer() { return srv.depthBuffer; }
		Render::HLSL::Texture2D<float4>& GetTransmittance() { return srv.transmittance; }
		Render::HLSL::Texture3D<float4>& GetInscatter() { return srv.inscatter; }
		Render::HLSL::Texture2D<float4>& GetIrradiance() { return srv.irradiance; }
		SkyData(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
