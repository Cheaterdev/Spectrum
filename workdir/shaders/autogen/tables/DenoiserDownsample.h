#pragma once
struct DenoiserDownsample_srv
{
	Texture2D<float4> color;
	Texture2D<float> depth;
};
struct DenoiserDownsample
{
	DenoiserDownsample_srv srv;
	Texture2D<float4> GetColor() { return srv.color; }
	Texture2D<float> GetDepth() { return srv.depth; }

};
 const DenoiserDownsample CreateDenoiserDownsample(DenoiserDownsample_srv srv)
{
	const DenoiserDownsample result = {srv
	};
	return result;
}
