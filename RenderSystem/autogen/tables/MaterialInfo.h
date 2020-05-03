#pragma once
namespace Table 
{
	struct MaterialInfo
	{
		struct CB
		{
			uint textureOffset;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		DynamicData data;
		uint& GetTextureOffset() { return cb.textureOffset; }
		DynamicData& GetData() { return data; }
		MaterialInfo(CB&cb) :cb(cb){}
	};
}
