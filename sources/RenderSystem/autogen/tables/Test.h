#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct Test
	{
		struct CB
		{
			float data[16];
		} &cb;
		Render::Bindless& bindless;
		float* GetData() { return cb.data; }
		Render::Bindless& GetTex() { return bindless; }
		Test(CB&cb,Render::Bindless &bindless) :cb(cb),bindless(bindless){}
	};
	#pragma pack(pop)
}
