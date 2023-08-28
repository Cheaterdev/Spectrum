export module HAL:Autogen.Tables.DrawBoxes;
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
	struct DrawBoxes
	{
		static constexpr SlotID ID = SlotID::DrawBoxes;
		HLSL::StructuredBuffer<float4> vertices;
		HLSL::StructuredBuffer<BoxInfo> input_meshes;
		HLSL::RWStructuredBuffer<uint> visible_meshes;
		HLSL::StructuredBuffer<float4>& GetVertices() { return vertices; }
		HLSL::StructuredBuffer<BoxInfo>& GetInput_meshes() { return input_meshes; }
		HLSL::RWStructuredBuffer<uint>& GetVisible_meshes() { return visible_meshes; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertices);
			compiler.compile(input_meshes);
			compiler.compile(visible_meshes);
		}
		struct Compiled
		{
			uint vertices; // StructuredBuffer<float4>
			uint input_meshes; // StructuredBuffer<BoxInfo>
			uint visible_meshes; // RWStructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
