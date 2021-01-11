#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Weights
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> areaTex;
			DX12::HLSL::Texture2D<float4> searchTex;
			DX12::HLSL::Texture2D<float4> edgesTex;
		} &srv;
		DX12::HLSL::Texture2D<float4>& GetAreaTex() { return srv.areaTex; }
		DX12::HLSL::Texture2D<float4>& GetSearchTex() { return srv.searchTex; }
		DX12::HLSL::Texture2D<float4>& GetEdgesTex() { return srv.edgesTex; }
		SMAA_Weights(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
