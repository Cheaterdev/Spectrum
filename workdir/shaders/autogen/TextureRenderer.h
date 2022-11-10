#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/TextureRenderer.h"
ConstantBuffer<TextureRenderer> pass_TextureRenderer: register( b2, space4);
const TextureRenderer CreateTextureRenderer()
{
	return pass_TextureRenderer;
}
#ifndef NO_GLOBAL
static const TextureRenderer textureRenderer_global = CreateTextureRenderer();
const TextureRenderer GetTextureRenderer(){ return textureRenderer_global; }
#endif
