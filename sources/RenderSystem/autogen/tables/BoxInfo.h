#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct BoxInfo
	{
		struct CB
		{
			uint node_offset;
			uint mesh_id;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetNode_offset() { return cb.node_offset; }
		uint& GetMesh_id() { return cb.mesh_id; }
		BoxInfo(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
