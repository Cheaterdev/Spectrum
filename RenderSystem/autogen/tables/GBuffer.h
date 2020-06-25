#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBuffer
	{
		using CB = Empty;
		struct SRV
		{
			Render::Handle albedo;
			Render::Handle normals;
			Render::Handle specular;
			Render::Handle depth;
		} &srv;
		using UAV = Empty;
		using SMP = Empty;
		Render::Handle& GetAlbedo() { return srv.albedo; }
		Render::Handle& GetNormals() { return srv.normals; }
		Render::Handle& GetSpecular() { return srv.specular; }
		Render::Handle& GetDepth() { return srv.depth; }
		GBuffer(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
