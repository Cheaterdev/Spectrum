#pragma once
struct GBuffer_srv
{
	Texture2D<float4> albedo;
	Texture2D<float4> normals;
	Texture2D<float4> specular;
	Texture2D<float> depth;
	Texture2D<float2> motion;
};
struct GBuffer
{
	GBuffer_srv srv;
	Texture2D<float4> GetAlbedo() { return srv.albedo; }
	Texture2D<float4> GetNormals() { return srv.normals; }
	Texture2D<float4> GetSpecular() { return srv.specular; }
	Texture2D<float> GetDepth() { return srv.depth; }
	Texture2D<float2> GetMotion() { return srv.motion; }

};
 const GBuffer CreateGBuffer(GBuffer_srv srv)
{
	const GBuffer result = {srv
	};
	return result;
}
