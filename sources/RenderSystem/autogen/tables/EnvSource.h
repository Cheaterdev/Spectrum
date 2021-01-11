#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct EnvSource
	{
		struct SRV
		{
			DX12::HLSL::TextureCube<float4> sourceTex;
		} &srv;
		DX12::HLSL::TextureCube<float4>& GetSourceTex() { return srv.sourceTex; }
		EnvSource(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
