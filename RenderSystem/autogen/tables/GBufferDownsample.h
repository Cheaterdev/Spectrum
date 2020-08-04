#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferDownsample
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle normals;
			Render::Handle depth;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetNormals() { return srv.normals; }
		Render::Handle& GetDepth() { return srv.depth; }
		GBufferDownsample(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
