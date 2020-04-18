#pragma once
namespace Table 
{
	struct buffer_test
	{
		struct CB
		{
			float arr[6];
		} &cb;
		struct SRV
		{
			Render::Handle data;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetData() { return srv.data; }
		float* GetArr() { return cb.arr; }
		buffer_test(CB&cb,SRV&srv) :cb(cb),srv(srv){}
	};
}
