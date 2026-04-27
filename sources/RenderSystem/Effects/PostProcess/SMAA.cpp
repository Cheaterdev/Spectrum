module Graphics:SMAA;

import <RenderSystem.h>;
import :FrameGraphContext;

import HAL;

#define STRINGIFY(x) #x
#define PPCAT_NX(A, B) A ## B

#define SMAA_TEXTURE(name) \
    EngineAsset<TextureAsset> name(PPCAT_NX(L,#name), [] { \
		return new TextureAsset(to_path(L"textures/" STRINGIFY(name) ".dds")); \
    })

namespace EngineAssets
{
	SMAA_TEXTURE(AreaTex);
	SMAA_TEXTURE(SearchTex);
}

SMAA::SMAA()
{
	area_tex   = EngineAssets::AreaTex.get_asset()->get_texture();
	search_tex = EngineAssets::SearchTex.get_asset()->get_texture();
}
