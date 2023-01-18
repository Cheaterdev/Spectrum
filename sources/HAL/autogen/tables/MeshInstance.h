#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshInstance
	{
		uint vertex_offset;
		uint index_offset;
		uint& GetVertex_offset() { return vertex_offset; }
		uint& GetIndex_offset() { return index_offset; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(vertex_offset);
			compiler.compile(index_offset);
		}
		using Compiled = MeshInstance;
		};
		#pragma pack(pop)
	}
