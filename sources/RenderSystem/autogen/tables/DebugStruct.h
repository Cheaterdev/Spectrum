#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct DebugStruct
	{
		struct CB
		{
			uint meshes_count;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetMeshes_count() { return cb.meshes_count; }
		DebugStruct(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
