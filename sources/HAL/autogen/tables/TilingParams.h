#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct TilingParams
	{
		HLSL::StructuredBuffer<uint2> tiles;
		HLSL::StructuredBuffer<uint2>& GetTiles() { return tiles; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(tiles);
		}
	};
	#pragma pack(pop)
}
