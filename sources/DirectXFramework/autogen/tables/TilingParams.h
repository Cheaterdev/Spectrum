#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct TilingParams
	{
		Render::HLSL::StructuredBuffer<uint2> tiles;
		Render::HLSL::StructuredBuffer<uint2>& GetTiles() { return tiles; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(tiles);
		}
	};
	#pragma pack(pop)
}
