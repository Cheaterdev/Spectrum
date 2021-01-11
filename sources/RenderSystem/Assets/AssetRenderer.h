#pragma once 
#include "Renderer/Renderer.h"
#include "Scene/Scene.h"
#include "Camera/Camera.h"
class G_Buffer;
class LightSystem;
class SSGI;
class SkyRender;
class SceneRenderWorkflow;
namespace DX12
{
	struct OVRContext;
}
class AssetRenderer : public Singleton<AssetRenderer>
{
	FrameGraph graph;
    UINT frame = 0;

        friend class Singleton<AssetRenderer>;
        main_renderer::ptr scene_renderer;
        DX12::FrameResourceManager frames;

        camera cam;
        mesh_renderer::ptr meshes_renderer;
        std::shared_ptr<SceneRenderWorkflow> rendering;

        MeshAssetInstance::ptr material_tester;
		MeshAssetInstance::ptr mesh_plane;

		Scene::ptr scene;
		
		AssetRenderer();

        mutex lock;
		std::shared_ptr<SSGI> ssgi;
		std::shared_ptr<SkyRender> sky;
		std::shared_ptr<DX12::OVRContext> vr_context ;
    public:

        virtual void draw(Scene::ptr scene, DX12::Texture::ptr result);
        virtual void draw(MaterialAsset::ptr m, DX12::Texture::ptr result);
		virtual void draw(scene_object::ptr scene, DX12::Texture::ptr result);

};