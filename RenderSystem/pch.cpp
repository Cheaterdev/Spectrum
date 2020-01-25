#include "pch.h"

namespace EngineAssets
{
	 EngineAsset<MeshAsset> material_tester(L"material_tester", [] {
		return (new MeshAsset(L"resources\\mitsuba\\mitsuba-sphere.obj"));
	});

	 EngineAsset<MeshAsset> plane(L"plane", [] {
		 return (new MeshAsset(L"resources\\plane.obj"));
	 });

	 EngineAsset<MeshAsset> axis(L"axis", [] {
		return (new MeshAsset(L"resources\\axis.obj"));
	});

	 EngineAsset<TextureAsset> best_fit_normals(L"best_fit_normals", [] {
		 return  new TextureAsset(L"resources\\best_fit_normals.dds");
	 });
	 EngineAsset<TextureAsset> missing_texture(L"missing_texture", [] {
		 return  new TextureAsset(L"resources\\missing.png");
	 });

	 EngineAsset<BinaryAsset> material_header(L"material_header", [] {
		 return  new BinaryAsset(L"shaders\\UniversalMaterial.hlsl");
	 });

	 EngineAsset<BinaryAsset> material_raytracing_header(L"material_header_raytracing", [] {
		 return  new BinaryAsset(L"shaders\\UniversalMaterialRaytracing.hlsl");
		 });



	 EngineAsset<BRDF> brdf(L"brdf", [] {
		 BRDF *brdf = new BRDF();
		 brdf->create_new();
		 return  brdf;
	 });
}