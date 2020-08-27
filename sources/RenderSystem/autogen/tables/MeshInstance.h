#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshInstance
	{
		struct CB
		{
			uint vertex_offset;
			uint index_offset;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetVertex_offset() { return cb.vertex_offset; }
		uint& GetIndex_offset() { return cb.index_offset; }
		MeshInstance(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
