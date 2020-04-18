#pragma once
namespace Table 
{
	struct MaterialInfo
	{
		using CB = DefaultCB;
		CB &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		Render::Bindless& bindless;
		Render::Bindless& GetTextures() { return bindless; }
		MaterialInfo(CB&cb,Render::Bindless &bindless) :cb(cb),bindless(bindless){}
	};
}
