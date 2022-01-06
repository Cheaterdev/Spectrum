#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct VoxelVisibility
	{
		Render::HLSL::Texture3D<uint> visibility;
		Render::HLSL::AppendStructuredBuffer<uint4> visible_tiles;
		Render::HLSL::Texture3D<uint>& GetVisibility() { return visibility; }
		Render::HLSL::AppendStructuredBuffer<uint4>& GetVisible_tiles() { return visible_tiles; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(visibility);
			compiler.compile(visible_tiles);
		}
	};
	#pragma pack(pop)
}
