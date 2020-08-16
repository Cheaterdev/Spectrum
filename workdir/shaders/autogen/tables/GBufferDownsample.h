#pragma once
struct GBufferDownsample_srv
{
	Texture2D normals;
	Texture2D depth;
};
struct GBufferDownsample
{
	GBufferDownsample_srv srv;
	Texture2D GetNormals() { return srv.normals; }
	Texture2D GetDepth() { return srv.depth; }
};
 const GBufferDownsample CreateGBufferDownsample(GBufferDownsample_srv srv)
{
	const GBufferDownsample result = {srv
	};
	return result;
}
