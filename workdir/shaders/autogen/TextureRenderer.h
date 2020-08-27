#ifndef SLOT_2
	#define SLOT_2
#else
	#error Slot 2 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/TextureRenderer.h"
Texture2D<float4> srv_2_0: register(t0, space2);
TextureRenderer CreateTextureRenderer()
{
	TextureRenderer result;
	result.srv.texture = srv_2_0;
	return result;
}
#ifndef NO_GLOBAL
static const TextureRenderer textureRenderer_global = CreateTextureRenderer();
const TextureRenderer GetTextureRenderer(){ return textureRenderer_global; }
#endif
