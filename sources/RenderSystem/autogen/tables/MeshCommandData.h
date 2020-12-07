#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MeshCommandData
	{
		struct CB
		{
			uint material_id;
			uint node_offset;
			GPUAddress mesh_cb;
			DrawIndexedArguments draw_commands;
		} &cb;
		uint& GetMaterial_id() { return cb.material_id; }
		uint& GetNode_offset() { return cb.node_offset; }
		GPUAddress& GetMesh_cb() { return cb.mesh_cb; }
		DrawIndexedArguments& GetDraw_commands() { return cb.draw_commands; }
		MeshCommandData(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
