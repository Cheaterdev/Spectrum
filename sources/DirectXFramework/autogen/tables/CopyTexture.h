#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct CopyTexture
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> srcTex;
		} &srv;
		Render::HLSL::Texture2D<float4>& GetSrcTex() { return srv.srcTex; }
		CopyTexture(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
