export module HAL:Autogen.Tables.GatherBoxes;
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
	struct GatherBoxes
	{
		static constexpr SlotID ID = SlotID::GatherBoxes;
		HLSL::AppendStructuredBuffer<BoxInfo> culledMeshes;
		HLSL::AppendStructuredBuffer<uint> visibleMeshes;
		HLSL::AppendStructuredBuffer<BoxInfo>& GetCulledMeshes() { return culledMeshes; }
		HLSL::AppendStructuredBuffer<uint>& GetVisibleMeshes() { return visibleMeshes; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(culledMeshes);
			compiler.compile(visibleMeshes);
		}
		struct Compiled
		{
			uint culledMeshes; // AppendStructuredBuffer<BoxInfo>
			uint visibleMeshes; // AppendStructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
