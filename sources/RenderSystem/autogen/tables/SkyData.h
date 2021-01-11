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
			DX12::HLSL::Texture2D<float> depthBuffer;
			DX12::HLSL::Texture2D<float4> transmittance;
			DX12::HLSL::Texture3D<float4> inscatter;
			DX12::HLSL::Texture2D<float4> irradiance;
		} &srv;
		float3& GetSunDir() { return cb.sunDir; }
		DX12::HLSL::Texture2D<float>& GetDepthBuffer() { return srv.depthBuffer; }
		DX12::HLSL::Texture2D<float4>& GetTransmittance() { return srv.transmittance; }
		DX12::HLSL::Texture3D<float4>& GetInscatter() { return srv.inscatter; }
		DX12::HLSL::Texture2D<float4>& GetIrradiance() { return srv.irradiance; }
		SkyData(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
	#pragma pack(pop)
}
