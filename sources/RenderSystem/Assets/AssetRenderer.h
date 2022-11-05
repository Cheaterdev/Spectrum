#pragma once
#include "Renderer/Renderer.h"

import Graphics;

class G_Buffer;
class LightSystem;
class SSGI;
class SkyRender;
class SceneRenderWorkflow;
namespace Graphics
{
	struct OVRContext;
}
class AssetRenderer : public Singleton<AssetRenderer>
{
    FrameGraph::Graph graph;
    UINT frame = 0;

        friend class Singleton<AssetRenderer>;
        main_renderer::ptr scene_renderer;
        HAL::FrameResourceManager frames;

        camera cam;
        mesh_renderer::ptr meshes_renderer;
        std::shared_ptr<SceneRenderWorkflow> rendering;

        MeshAssetInstance::ptr material_tester;
		MeshAssetInstance::ptr mesh_plane;

		Scene::ptr scene;
		
		AssetRenderer();

        std::mutex lock;
		std::shared_ptr<SSGI> ssgi;
		std::shared_ptr<SkyRender> sky;
		std::shared_ptr<Graphics::OVRContext> vr_context ;
    public:

        virtual void draw(Scene::ptr scene, HAL::Texture::ptr result);
        virtual void draw(MaterialAsset::ptr m, HAL::Texture::ptr result);
		virtual void draw(scene_object::ptr scene, HAL::Texture::ptr result);

};