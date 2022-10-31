#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Weights
	{
		HLSL::Texture2D<float4> areaTex;
		HLSL::Texture2D<float4> searchTex;
		HLSL::Texture2D<float4> edgesTex;
		HLSL::Texture2D<float4>& GetAreaTex() { return areaTex; }
		HLSL::Texture2D<float4>& GetSearchTex() { return searchTex; }
		HLSL::Texture2D<float4>& GetEdgesTex() { return edgesTex; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(areaTex);
			compiler.compile(searchTex);
			compiler.compile(edgesTex);
		}
	};
	#pragma pack(pop)
}