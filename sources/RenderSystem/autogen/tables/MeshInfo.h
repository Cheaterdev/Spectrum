#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshInfo
	{
		struct CB
		{
			uint node_offset;
			uint vertex_offset;
		} &cb;
		uint& GetNode_offset() { return cb.node_offset; }
		uint& GetVertex_offset() { return cb.vertex_offset; }
		MeshInfo(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
