#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Weights
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> areaTex;
			Render::HLSL::Texture2D<float4> searchTex;
			Render::HLSL::Texture2D<float4> edgesTex;
		} &srv;
		Render::HLSL::Texture2D<float4>& GetAreaTex() { return srv.areaTex; }
		Render::HLSL::Texture2D<float4>& GetSearchTex() { return srv.searchTex; }
		Render::HLSL::Texture2D<float4>& GetEdgesTex() { return srv.edgesTex; }
		SMAA_Weights(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
