export module Graphics:AssetRenderer;
import Core;
import HAL;
import :Texture;
 import :MeshRenderer;
 import :Sky;



class SceneRenderWorkflow;

export  class AssetRenderer : public Singleton<AssetRenderer>
{
    FrameGraph::Graph graph;
    UINT frame = 0;

        friend class Singleton<AssetRenderer>;
        main_renderer::ptr scene_renderer;
     //   HAL::FrameResourceManager frames;

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


// Generic offscreen "render a scene into a target texture" engine. Owns its own
// FrameGraph so it can run alongside the main UI graph (which only fits one main
// pipeline). Copied from AssetRenderer's implementation; AssetRenderer will later
// be rewritten to build on this. Not a Singleton — each user owns an instance
// (own graph / scene / camera).
export class SceneTextureRenderer
{
        FrameGraph::Graph graph;
        UINT frame = 0;

        main_renderer::ptr scene_renderer;
        camera cam;
        mesh_renderer::ptr meshes_renderer;
        std::shared_ptr<SceneRenderWorkflow> rendering;

        MeshAssetInstance::ptr mesh_plane;
        MeshAssetInstance::ptr material_tester; // test mesh for material previews

        Scene::ptr scene;
        std::mutex lock;

        // Orbit camera around the auto-fit center: yaw/pitch + zoom distance.
        vec2  m_orbit = { 0.785398f, 0.35f }; // ~pi/4 yaw, slight pitch
        float m_zoom  = 1.0f;

    public:
        SceneTextureRenderer();

        camera& get_camera() { return cam; }

        // Drag to orbit, wheel to zoom (forwarded from the preview widget).
        void orbit(vec2 delta);
        void zoom(float amount);

        void draw(Scene::ptr scene, HAL::Texture::ptr result);
        void draw(scene_object::ptr obj, HAL::Texture::ptr result);
        void draw(MaterialAsset::ptr m, HAL::Texture::ptr result);
};