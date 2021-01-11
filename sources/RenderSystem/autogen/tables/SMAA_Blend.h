#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Blend
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> blendTex;
		} &srv;
		DX12::HLSL::Texture2D<float4>& GetBlendTex() { return srv.blendTex; }
		SMAA_Blend(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
