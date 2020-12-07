#pragma once
struct GBufferQuality_srv
{
	Texture2D<float4> ref;
};
struct GBufferQuality
{
	GBufferQuality_srv srv;
	Texture2D<float4> GetRef() { return srv.ref; }

};
 const GBufferQuality CreateGBufferQuality(GBufferQuality_srv srv)
{
	const GBufferQuality result = {srv
	};
	return result;
}
