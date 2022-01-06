#pragma once
#include "BoxInfo.h"
namespace Table 
{
	#pragma pack(push, 1)
	struct GatherMeshesBoxes
	{
		Render::HLSL::StructuredBuffer<BoxInfo> input_meshes;
		Render::HLSL::StructuredBuffer<uint> visible_boxes;
		Render::HLSL::AppendStructuredBuffer<uint> visibleMeshes;
		Render::HLSL::AppendStructuredBuffer<uint> invisibleMeshes;
		Render::HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return input_meshes; }
		Render::HLSL::StructuredBuffer<uint>& GetVisible_boxes() { return visible_boxes; }
		Render::HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return visibleMeshes; }
		Render::HLSL::AppendStructuredBuffer<uint>& GetInvisibleMeshes() { return invisibleMeshes; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(input_meshes);
			compiler.compile(visible_boxes);
			compiler.compile(visibleMeshes);
			compiler.compile(invisibleMeshes);
		}
	};
	#pragma pack(pop)
}
