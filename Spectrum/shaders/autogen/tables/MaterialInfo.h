#pragma once
struct MaterialInfo
{
	MaterialInfo_cb cb;
	Texture2D GetTextures(int i) { return bindless[i]; }
};
 const MaterialInfo CreateMaterialInfo(MaterialInfo_cb cb)
{
	const MaterialInfo result = {cb
	};
	return result;
}
