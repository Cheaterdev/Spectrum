#pragma once
struct GBufferQuality_srv
{
	Texture2D ref;
};
struct GBufferQuality
{
	GBufferQuality_srv srv;
	Texture2D GetRef() { return srv.ref; }
};
 const GBufferQuality CreateGBufferQuality(GBufferQuality_srv srv)
{
	const GBufferQuality result = {srv
	};
	return result;
}
