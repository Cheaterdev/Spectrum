#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshInfo
	{
		uint vertex_offset_local;
		uint meshlet_offset_local;
		uint node_offset;
		uint meshlet_count;
		uint& GetVertex_offset_local() { return vertex_offset_local; }
		uint& GetMeshlet_offset_local() { return meshlet_offset_local; }
		uint& GetNode_offset() { return node_offset; }
		uint& GetMeshlet_count() { return meshlet_count; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertex_offset_local);
			compiler.compile(meshlet_offset_local);
			compiler.compile(node_offset);
			compiler.compile(meshlet_count);
		}
		using Compiled = MeshInfo;
		};
		#pragma pack(pop)
	}
