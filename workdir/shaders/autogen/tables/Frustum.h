#pragma once
struct Frustum_cb
{
	float4 planes[6]; // float4
};
struct Frustum
{
	Frustum_cb cb;
	float4 GetPlanes(int i) { return cb.planes[i]; }

};
 const Frustum CreateFrustum(Frustum_cb cb)
{
	const Frustum result = {cb
	};
	return result;
}
