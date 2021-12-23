#pragma once
struct Countour_cb
{
	float4 color; // float4
};
struct Countour_srv
{
	uint tex; // Texture2D<float4>
};
struct Countour
{
	Countour_cb cb;
	Countour_srv srv;
	Texture2D<float4> GetTex() { return ResourceDescriptorHeap[srv.tex]; }
	float4 GetColor() { return cb.color; }

};
 const Countour CreateCountour(Countour_cb cb,Countour_srv srv)
{
	const Countour result = {cb,srv
	};
	return result;
}
