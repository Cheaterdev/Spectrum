#pragma once
namespace Table 
{
	struct MeshData
	{
		struct CB
		{
			uint material_id;
			uint4 mesh_cb;
			uint4 draw_commands;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetMaterial_id() { return cb.material_id; }
		uint4& GetMesh_cb() { return cb.mesh_cb; }
		uint4& GetDraw_commands() { return cb.draw_commands; }
		MeshData(CB&cb) :cb(cb){}
	};
}
