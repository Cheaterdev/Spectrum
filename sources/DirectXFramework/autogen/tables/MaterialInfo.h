#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MaterialInfo
	{
		struct CB
		{
			uint textureOffset;
		} &cb;
		DynamicData data;
		uint& GetTextureOffset() { return cb.textureOffset; }
		DynamicData& GetData() { return data; }
		MaterialInfo(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
