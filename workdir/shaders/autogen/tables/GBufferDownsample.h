#pragma once
struct GBufferDownsample_srv
{
	Texture2D<float4> normals;
	Texture2D<float> depth;
};
struct GBufferDownsample
{
	GBufferDownsample_srv srv;
	Texture2D<float4> GetNormals() { return srv.normals; }
	Texture2D<float> GetDepth() { return srv.depth; }

};
 const GBufferDownsample CreateGBufferDownsample(GBufferDownsample_srv srv)
{
	const GBufferDownsample result = {srv
	};
	return result;
}
