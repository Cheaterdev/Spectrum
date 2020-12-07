#pragma once
struct EnvSource_srv
{
	TextureCube<float4> sourceTex;
};
struct EnvSource
{
	EnvSource_srv srv;
	TextureCube<float4> GetSourceTex() { return srv.sourceTex; }

};
 const EnvSource CreateEnvSource(EnvSource_srv srv)
{
	const EnvSource result = {srv
	};
	return result;
}
