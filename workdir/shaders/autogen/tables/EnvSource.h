#pragma once
struct EnvSource_srv
{
	TextureCube sourceTex;
};
struct EnvSource
{
	EnvSource_srv srv;
	TextureCube GetSourceTex() { return srv.sourceTex; }
};
 const EnvSource CreateEnvSource(EnvSource_srv srv)
{
	const EnvSource result = {srv
	};
	return result;
}
