#pragma once
#include "Asset.h"
#include "MeshAsset.h"
#include "BinaryAsset.h"
#include "TextureAsset.h"
#include "Lighting/BRDF.h"

namespace EngineAssets
{
	extern EngineAsset<MeshAsset> material_tester;
	extern EngineAsset<MeshAsset> axis;
	extern EngineAsset<MeshAsset> plane;
	extern EngineAsset<BinaryAsset> material_header;
	extern EngineAsset<BinaryAsset> material_raytracing_header;


	extern EngineAsset<TextureAsset> best_fit_normals;
	extern EngineAsset<TextureAsset> missing_texture;

	extern EngineAsset<BRDF> brdf;

}