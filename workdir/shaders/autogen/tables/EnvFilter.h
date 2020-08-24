#pragma once
struct EnvFilter_cb
{
	uint4 face;
	float4 scaler;
	uint4 size;
};
struct EnvFilter
{
	EnvFilter_cb cb;
	uint4 GetFace() { return cb.face; }
	float4 GetScaler() { return cb.scaler; }
	uint4 GetSize() { return cb.size; }

};
 const EnvFilter CreateEnvFilter(EnvFilter_cb cb)
{
	const EnvFilter result = {cb
	};
	return result;
}
