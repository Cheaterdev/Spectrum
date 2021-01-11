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
		DX12::Bindless& bindless;
		float* GetData() { return cb.data; }
		DX12::Bindless& GetTex() { return bindless; }
		Test(CB&cb,DX12::Bindless &bindless) :cb(cb),bindless(bindless){}
	};
	#pragma pack(pop)
}
