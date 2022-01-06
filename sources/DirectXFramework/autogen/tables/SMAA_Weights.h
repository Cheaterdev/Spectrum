#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Weights
	{
		Render::HLSL::Texture2D<float4> areaTex;
		Render::HLSL::Texture2D<float4> searchTex;
		Render::HLSL::Texture2D<float4> edgesTex;
		Render::HLSL::Texture2D<float4>& GetAreaTex() { return areaTex; }
		Render::HLSL::Texture2D<float4>& GetSearchTex() { return searchTex; }
		Render::HLSL::Texture2D<float4>& GetEdgesTex() { return edgesTex; }
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
