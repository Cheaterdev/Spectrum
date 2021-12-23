#pragma once
struct GBufferQuality_srv
{
	uint ref; // Texture2D<float4>
};
struct GBufferQuality
{
	GBufferQuality_srv srv;
	Texture2D<float4> GetRef() { return ResourceDescriptorHeap[srv.ref]; }

};
 const GBufferQuality CreateGBufferQuality(GBufferQuality_srv srv)
{
	const GBufferQuality result = {srv
	};
	return result;
}
