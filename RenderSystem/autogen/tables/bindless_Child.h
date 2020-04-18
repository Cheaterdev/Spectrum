#pragma once
namespace Table 
{
	struct bindless_Child
	{
		struct CB
		{
			float data[16];
		} &cb;
		struct SRV
		{
			Render::Handle albed;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Bindless& bindless;
		float* GetData() { return cb.data; }
		Render::Handle& GetAlbed() { return srv.albed; }
		Render::Bindless& GetTex() { return bindless; }
		bindless_Child(CB&cb,SRV&srv,Render::Bindless &bindless) :cb(cb),srv(srv),bindless(bindless){}
	};
}
