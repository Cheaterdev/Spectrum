#pragma once
struct MaterialInfo_cb
{
	MaterialCB data;
	uint textureOffset;
};
struct MaterialInfo
{
	MaterialInfo_cb cb;
	MaterialCB GetData() { return cb.data; }
	uint GetTextureOffset() { return cb.textureOffset; }

};
 const MaterialInfo CreateMaterialInfo(MaterialInfo_cb cb)
{
	const MaterialInfo result = {cb
	};
	return result;
}
