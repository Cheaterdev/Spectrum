#pragma once
namespace Table 
{
	struct Test
	{
		struct CB
		{
			float data[16];
		} &cb;
		using SRV = Empty;
		using UAV = Empty;
		using SMP = Empty;
		Render::Bindless& bindless;
		float* GetData() { return cb.data; }
		Render::Bindless& GetTex() { return bindless; }
		Test(CB&cb,Render::Bindless &bindless) :cb(cb),bindless(bindless){}
	};
}
