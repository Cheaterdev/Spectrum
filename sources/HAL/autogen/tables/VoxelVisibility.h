#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelVisibility
	{
		HLSL::Texture3D<uint> visibility;
		HLSL::AppendStructuredBuffer<uint4> visible_tiles;
		HLSL::Texture3D<uint>& GetVisibility() { return visibility; }
		HLSL::AppendStructuredBuffer<uint4>& GetVisible_tiles() { return visible_tiles; }
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
