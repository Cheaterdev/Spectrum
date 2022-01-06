#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshInfo
	{
		uint node_offset;
		uint vertex_offset;
		uint meshlet_offset;
		uint meshlet_count;
		uint meshlet_unique_offset;
		uint meshlet_vertex_offset;
		uint& GetNode_offset() { return node_offset; }
		uint& GetVertex_offset() { return vertex_offset; }
		uint& GetMeshlet_offset() { return meshlet_offset; }
		uint& GetMeshlet_count() { return meshlet_count; }
		uint& GetMeshlet_unique_offset() { return meshlet_unique_offset; }
		uint& GetMeshlet_vertex_offset() { return meshlet_vertex_offset; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(node_offset);
			compiler.compile(vertex_offset);
			compiler.compile(meshlet_offset);
			compiler.compile(meshlet_count);
			compiler.compile(meshlet_unique_offset);
			compiler.compile(meshlet_vertex_offset);
		}
	};
	#pragma pack(pop)
}
