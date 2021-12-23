#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/TextureRenderer.h"
Texture2D<float4> srv_4_0: register(t0, space4);
struct Pass_TextureRenderer
{
uint srv_0;
};
ConstantBuffer<Pass_TextureRenderer> pass_TextureRenderer: register( b2, space4);
const TextureRenderer CreateTextureRenderer()
{
	TextureRenderer result;
	result.srv.texture = (pass_TextureRenderer.srv_0 );
	return result;
}
#ifndef NO_GLOBAL
static const TextureRenderer textureRenderer_global = CreateTextureRenderer();
const TextureRenderer GetTextureRenderer(){ return textureRenderer_global; }
#endif
