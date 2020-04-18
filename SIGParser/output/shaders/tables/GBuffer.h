#pragma once
struct GBuffer_srv
{
	Texture2D albedo;
	Texture2D normals;
	Texture2D specular;
	Texture2D depth;
};
struct GBuffer
{
	GBuffer_srv srv;
	Texture2D GetAlbedo() { return srv.albedo; }
	Texture2D GetNormals() { return srv.normals; }
	Texture2D GetSpecular() { return srv.specular; }
	Texture2D GetDepth() { return srv.depth; }
};
 const GBuffer CreateGBuffer(GBuffer_srv srv)
{
	const GBuffer result = {srv
	};
	return result;
}
