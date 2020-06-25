#pragma once
namespace Table 
{
	struct MaterialData
	{
		struct CB
		{
			uint4 material_cb;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint4& GetMaterial_cb() { return cb.material_cb; }
		MaterialData(CB&cb) :cb(cb){}
	};
}
