export module HAL:Autogen.Tables.SMAA_Weights;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct SMAA_Weights
	{
		static constexpr SlotID ID = SlotID::SMAA_Weights;
		HLSL::Texture2D<float4> areaTex;
		HLSL::Texture2D<float4> searchTex;
		HLSL::Texture2D<float4> edgesTex;
		HLSL::Texture2D<float4>& GetAreaTex() { return areaTex; }
		HLSL::Texture2D<float4>& GetSearchTex() { return searchTex; }
		HLSL::Texture2D<float4>& GetEdgesTex() { return edgesTex; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(areaTex);
			compiler.compile(searchTex);
			compiler.compile(edgesTex);
		}
		struct Compiled
		{
			uint areaTex; // Texture2D<float4>
			uint searchTex; // Texture2D<float4>
			uint edgesTex; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
