#pragma once
struct SkyDownsample_cb
{
	float4 scaler;
	uint4 size;
};
struct SkyDownsample_srv
{
	TextureCube sourceTex;
};
struct SkyDownsample
{
	SkyDownsample_cb cb;
	SkyDownsample_srv srv;
	TextureCube GetSourceTex() { return srv.sourceTex; }
	float4 GetScaler() { return cb.scaler; }
	uint4 GetSize() { return cb.size; }
};
 const SkyDownsample CreateSkyDownsample(SkyDownsample_cb cb,SkyDownsample_srv srv)
{
	const SkyDownsample result = {cb,srv
	};
	return result;
}
