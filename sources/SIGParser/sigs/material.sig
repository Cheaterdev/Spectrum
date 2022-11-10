[Bind = DefaultLayout::MaterialData]
struct MaterialInfo
{
	[dynamic] MaterialCB data;
    Texture2D<float4> textures[];
}
