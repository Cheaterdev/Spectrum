#pragma once
struct EnvSource_srv
{
	uint sourceTex; // TextureCube<float4>
};
struct EnvSource
{
	EnvSource_srv srv;
	TextureCube<float4> GetSourceTex() { return ResourceDescriptorHeap[srv.sourceTex]; }

};
 const EnvSource CreateEnvSource(EnvSource_srv srv)
{
	const EnvSource result = {srv
	};
	return result;
}
