#pragma once
struct MaterialInfo
{
	MaterialCB data; // MaterialCB
	uint textureOffset; // uint
	MaterialCB GetData() { return data; }
	uint GetTextureOffset() { return textureOffset; }
};
