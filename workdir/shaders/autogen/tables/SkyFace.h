#pragma once
struct SkyFace_cb
{
	uint face;
};
struct SkyFace
{
	SkyFace_cb cb;
	uint GetFace() { return cb.face; }

};
 const SkyFace CreateSkyFace(SkyFace_cb cb)
{
	const SkyFace result = {cb
	};
	return result;
}
