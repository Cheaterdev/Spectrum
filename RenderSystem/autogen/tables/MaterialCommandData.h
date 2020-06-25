#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MaterialCommandData
	{
		struct CB
		{
			uint pipeline_id;
			GPUAddress material_cb;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetPipeline_id() { return cb.pipeline_id; }
		GPUAddress& GetMaterial_cb() { return cb.material_cb; }
		MaterialCommandData(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
