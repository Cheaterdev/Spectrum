#pragma once

#include "Lighting/BRDF.h"
#include "MeshAsset.h"
#include "TextureAsset.h"
#include "BinaryAsset.h"

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