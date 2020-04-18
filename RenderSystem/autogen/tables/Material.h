#pragma once
namespace Table 
{
	struct Material
	{
		struct CB
		{
			uint textureOffset;
			uint nodeOffset;
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		uint& GetTextureOffset() { return cb.textureOffset; }
		uint& GetNodeOffset() { return cb.nodeOffset; }
		Material(CB&cb) :cb(cb){}
	};
}
