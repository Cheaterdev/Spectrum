# Per-node preview capture for the material graph editor.
#
# A material's generated body is compiled once into this compute PSO
# instead of the production pixel PSO. The codegen (MaterialContext,
# see Materials/Values.cpp) injects one extra write per graph node right
# after that node's value is computed, so a single dispatch fills every
# node's preview slice at once instead of needing one PSO per node.
#
# Deliberately NOT sharing MaterialInfo/DefaultLayout::MaterialData: this
# keeps the preview path fully decoupled from the production material
# root signature used by every object in the real scene.
[Bind = DefaultLayout::Instance0]
struct MaterialPreviewInfo
{
	[dynamic] MaterialCB data;
	Texture2D<float4> textures[];
	RWTexture2DArray<float4> results;
}

ComputePSO MaterialPreview
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = material_preview;
}
