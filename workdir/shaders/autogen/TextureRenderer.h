#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/TextureRenderer.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_TextureRenderer: register( b2, space4);
 ConstantBuffer<TextureRenderer> CreateTextureRenderer()
{
	return ResourceDescriptorHeap[pass_TextureRenderer.offset];
}
#ifndef NO_GLOBAL
static const TextureRenderer textureRenderer_global = CreateTextureRenderer();
const TextureRenderer GetTextureRenderer(){ return textureRenderer_global; }
#endif
