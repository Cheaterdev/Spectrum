#pragma once
struct Material_cb
{
	uint textureOffset;
	uint nodeOffset;
};
struct Material
{
	Material_cb cb;
	uint GetTextureOffset() { return cb.textureOffset; }
	uint GetNodeOffset() { return cb.nodeOffset; }
};
 const Material CreateMaterial(Material_cb cb)
{
	const Material result = {cb
	};
	return result;
}
