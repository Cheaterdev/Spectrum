export module Graphics:AssetRenderer;

import Singleton;
import HAL;
 import :MeshRenderer;
 import :Sky;



class SceneRenderWorkflow;

export  class AssetRenderer : public Singleton<AssetRenderer>
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
	//	std::shared_ptr<SSGI> ssgi;
		std::shared_ptr<SkyRender> sky;

    public:

        virtual void draw(Scene::ptr scene, HAL::Texture::ptr result);
        virtual void draw(MaterialAsset::ptr m, HAL::Texture::ptr result);
		virtual void draw(scene_object::ptr scene, HAL::Texture::ptr result);

};