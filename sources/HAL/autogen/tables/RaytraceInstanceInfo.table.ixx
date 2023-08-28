export module HAL:Autogen.Tables.RaytraceInstanceInfo;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.mesh_vertex_input;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct RaytraceInstanceInfo
	{
		static constexpr SlotID ID = SlotID::RaytraceInstanceInfo;
		HLSL::StructuredBuffer<mesh_vertex_input> vertexes;
		HLSL::StructuredBuffer<uint> indices;
		HLSL::StructuredBuffer<mesh_vertex_input>& GetVertexes() { return vertexes; }
		HLSL::StructuredBuffer<uint>& GetIndices() { return indices; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertexes);
			compiler.compile(indices);
		}
		struct Compiled
		{
			uint vertexes; // StructuredBuffer<mesh_vertex_input>
			uint indices; // StructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
