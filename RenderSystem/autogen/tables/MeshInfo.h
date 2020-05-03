#pragma once
namespace Table 
{
	struct MeshInfo
	{
		struct CB
		{
			uint node_offset;
			uint vertex_offset;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetNode_offset() { return cb.node_offset; }
		uint& GetVertex_offset() { return cb.vertex_offset; }
		MeshInfo(CB&cb) :cb(cb){}
	};
}
