#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/TextureRenderer.h"
Texture2D<float4> srv_3_0: register(t0, space3);
TextureRenderer CreateTextureRenderer()
{
	TextureRenderer result;
	result.srv.texture = srv_3_0;
	return result;
}
#ifndef NO_GLOBAL
static const TextureRenderer textureRenderer_global = CreateTextureRenderer();
const TextureRenderer GetTextureRenderer(){ return textureRenderer_global; }
#endif
