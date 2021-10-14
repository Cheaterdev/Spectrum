#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Blend
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> blendTex;
		} &srv;
		Render::HLSL::Texture2D<float4>& GetBlendTex() { return srv.blendTex; }
		SMAA_Blend(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
