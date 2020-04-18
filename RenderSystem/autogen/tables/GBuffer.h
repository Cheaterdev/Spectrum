#pragma once
namespace Table 
{
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
}
