export module HAL:Autogen.Tables.GatherMeshesBoxes;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.BoxInfo;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct GatherMeshesBoxes
	{
		static constexpr SlotID ID = SlotID::GatherMeshesBoxes;
		HLSL::StructuredBuffer<BoxInfo> input_meshes;
		HLSL::StructuredBuffer<uint> visible_boxes;
		HLSL::AppendStructuredBuffer<uint> visibleMeshes;
		HLSL::AppendStructuredBuffer<uint> invisibleMeshes;
		HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return input_meshes; }
		HLSL::StructuredBuffer<uint>& GetVisible_boxes() { return visible_boxes; }
		HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return visibleMeshes; }
		HLSL::AppendStructuredBuffer<uint>& GetInvisibleMeshes() { return invisibleMeshes; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(input_meshes);
			compiler.compile(visible_boxes);
			compiler.compile(visibleMeshes);
			compiler.compile(invisibleMeshes);
		}
		struct Compiled
		{
			uint input_meshes; // StructuredBuffer<BoxInfo>
			uint visible_boxes; // StructuredBuffer<uint>
			uint visibleMeshes; // AppendStructuredBuffer<uint>
			uint invisibleMeshes; // AppendStructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
