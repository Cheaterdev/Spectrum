export module HAL:Autogen.Tables.VoxelVisibility;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelVisibility
	{
		static constexpr SlotID ID = SlotID::VoxelVisibility;
		HLSL::Texture3D<uint> visibility;
		HLSL::AppendStructuredBuffer<uint4> visible_tiles;
		HLSL::Texture3D<uint>& GetVisibility() { return visibility; }
		HLSL::AppendStructuredBuffer<uint4>& GetVisible_tiles() { return visible_tiles; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(visibility);
			compiler.compile(visible_tiles);
		}
		struct Compiled
		{
			uint visibility; // Texture3D<uint>
			uint visible_tiles; // AppendStructuredBuffer<uint4>
		};
	};
	#pragma pack(pop)
}
