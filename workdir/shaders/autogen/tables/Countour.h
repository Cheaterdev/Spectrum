#pragma once
struct Countour_cb
{
	float4 color;
};
struct Countour_srv
{
	Texture2D<float4> tex;
};
struct Countour
{
	Countour_cb cb;
	Countour_srv srv;
	float4 GetColor() { return cb.color; }
	Texture2D<float4> GetTex() { return srv.tex; }
};
 const Countour CreateCountour(Countour_cb cb,Countour_srv srv)
{
	const Countour result = {cb,srv
	};
	return result;
}
