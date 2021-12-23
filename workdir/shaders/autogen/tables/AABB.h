#pragma once
struct AABB_cb
{
	float4 min; // float4
	float4 max; // float4
};
struct AABB
{
	AABB_cb cb;
	float4 GetMin() { return cb.min; }
	float4 GetMax() { return cb.max; }

};
 const AABB CreateAABB(AABB_cb cb)
{
	const AABB result = {cb
	};
	return result;
}
