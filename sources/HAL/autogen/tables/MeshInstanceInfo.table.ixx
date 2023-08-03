export module HAL:Autogen.Tables.MeshInstanceInfo;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.Meshlet;
import :Autogen.Tables.MeshletCullData;
import :Autogen.Tables.mesh_vertex_input;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct MeshInstanceInfo
	{
		static constexpr SlotID ID = SlotID::MeshInstanceInfo;
		HLSL::StructuredBuffer<mesh_vertex_input> vertexes;
		HLSL::StructuredBuffer<uint> indices;
		HLSL::StructuredBuffer<Meshlet> meshlets;
		HLSL::StructuredBuffer<MeshletCullData> meshletCullData;
		HLSL::StructuredBuffer<uint> unique_indices;
		HLSL::StructuredBuffer<uint> primitive_indices;
		HLSL::StructuredBuffer<mesh_vertex_input>& GetVertexes() { return vertexes; }
		HLSL::StructuredBuffer<uint>& GetIndices() { return indices; }
		HLSL::StructuredBuffer<Meshlet>& GetMeshlets() { return meshlets; }
		HLSL::StructuredBuffer<MeshletCullData>& GetMeshletCullData() { return meshletCullData; }
		HLSL::StructuredBuffer<uint>& GetUnique_indices() { return unique_indices; }
		HLSL::StructuredBuffer<uint>& GetPrimitive_indices() { return primitive_indices; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertexes);
			compiler.compile(indices);
			compiler.compile(meshlets);
			compiler.compile(meshletCullData);
			compiler.compile(unique_indices);
			compiler.compile(primitive_indices);
		}
		struct Compiled
		{
			uint vertexes; // StructuredBuffer<mesh_vertex_input>
			uint indices; // StructuredBuffer<uint>
			uint meshlets; // StructuredBuffer<Meshlet>
			uint meshletCullData; // StructuredBuffer<MeshletCullData>
			uint unique_indices; // StructuredBuffer<uint>
			uint primitive_indices; // StructuredBuffer<uint>
		};
	};
	#pragma pack(pop)
}
