#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct TilingParams
	{
		static constexpr SlotID ID = SlotID::TilingParams;
		HLSL::StructuredBuffer<uint2> tiles;
		HLSL::StructuredBuffer<uint2>& GetTiles() { return tiles; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(tiles);
		}
		struct Compiled
		{
			uint tiles; // StructuredBuffer<uint2>
		};
	};
	#pragma pack(pop)
}
