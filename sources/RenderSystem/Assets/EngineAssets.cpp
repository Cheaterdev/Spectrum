module Graphics:EngineAssets;
import :Asset;
import :MeshAsset;
import :BinaryAsset;
import :TextureAsset;

namespace EngineAssets
{
	EngineAsset<MeshAsset> material_tester(L"material_tester", [] {
		return (new MeshAsset(to_path(L"resources\\mitsuba\\mitsuba-sphere.obj").wstring()));
		});

	EngineAsset<MeshAsset> plane(L"plane", [] {
		return (new MeshAsset(to_path(L"resources\\plane.obj").wstring()));
		});

	EngineAsset<MeshAsset> axis(L"axis", [] {
		return (new MeshAsset(to_path(L"resources\\axis.obj").wstring()));
		});

	EngineAsset<TextureAsset> best_fit_normals(L"best_fit_normals", [] {
		return  new TextureAsset(to_path(L"resources\\best_fit_normals.dds").wstring());
		});
	EngineAsset<TextureAsset> missing_texture(L"missing_texture", [] {
		return  new TextureAsset(to_path(L"resources\\missing.png").wstring());
		});

	EngineAsset<BinaryAsset> material_header(L"material_header", [] {
		return  new BinaryAsset(to_path(L"shaders\\materials\\universal_material.hlsl").wstring());
		});

	EngineAsset<BinaryAsset> material_raytracing_header(L"material_header_raytracing", [] {
		return  new BinaryAsset(to_path(L"shaders\\rtx\\universal_material_raytracing.hlsl").wstring());
		});

	EngineAsset<BinaryAsset> material_preview_header(L"material_header_preview", [] {
		return  new BinaryAsset(to_path(L"shaders\\materials\\universal_material_preview.hlsl").wstring());
		});

	EngineAsset<BinaryAsset> material_preview_3d_header(L"material_header_preview_3d", [] {
		return  new BinaryAsset(to_path(L"shaders\\materials\\material_preview_3d.hlsl").wstring());
		});





}
