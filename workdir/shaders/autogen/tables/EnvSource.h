#pragma once
struct EnvSource
{
	uint sourceTex; // TextureCube<float4>
	TextureCube<float4> GetSourceTex() { return ResourceDescriptorHeap[sourceTex]; }
};
