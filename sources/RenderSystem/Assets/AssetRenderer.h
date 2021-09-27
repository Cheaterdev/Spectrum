class G_Buffer;
class LightSystem;
class SSGI;
class SkyRender;
class SceneRenderWorkflow;
namespace Render
{
	struct OVRContext;
}
class AssetRenderer : public Singleton<AssetRenderer>
{
    FrameGraph::Graph graph;
    UINT frame = 0;

        friend class Singleton<AssetRenderer>;
        main_renderer::ptr scene_renderer;
        Render::FrameResourceManager frames;

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
		std::shared_ptr<Render::OVRContext> vr_context ;
    public:

        virtual void draw(Scene::ptr scene, Render::Texture::ptr result);
        virtual void draw(MaterialAsset::ptr m, Render::Texture::ptr result);
		virtual void draw(scene_object::ptr scene, Render::Texture::ptr result);

};