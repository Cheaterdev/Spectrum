export module Graphics:BRDF;
import <RenderSystem.h>;
import :TextureAsset;
import :Asset;


export class BRDF: public TextureAsset
{
public:
	void create_new();
private:

	SERIALIZE();
};

export namespace EngineAssets
{
	extern EngineAsset<BRDF> brdf;
}
