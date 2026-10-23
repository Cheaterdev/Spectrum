export module Graphics:AssetRenderer;
import Core;
import HAL;
import :Texture;
 import :MeshRenderer;
 import :Sky;



class SceneRenderWorkflow;

// What the material editor's live preview shows the material on and under. Variables, so
// the Properties panel lists them and the editor panel builds its rows from them.
export enum class PreviewShape
{
    MaterialTester,   // the Mitsuba test object: a ball inside a glass case
    Sphere,
    RoundedCube,
    Slab,             // a thick plate: glass, absorption, subsurface depth
    Cloth,            // a draped sheet: sheen, anisotropy
};

export enum class PreviewEnvironment
{
    Sky,
    Studio,           // softboxes on a dark backdrop (sky/studio.hlsl)
};

export enum class PreviewLobes
{
    All,
    WithoutLobe,      // the material's extra lobe (coat, sheen, ...) switched off
    LobeDifference,   // what that lobe adds, as |with - without| x 4
};

export class PreviewSettings : public VariableContext
{
public:
    PreviewSettings() : VariableContext(L"Preview Settings") {}

    Variable<PreviewShape>       shape         = { PreviewShape::MaterialTester, "Shape", this };
    // Scene units matter to transmission depth and subsurface radius; the shapes are about 2 units across.
    Variable<float>              scale         = { 1.0f, "Scale", this, 0.05f, 10.0f };
    Variable<PreviewEnvironment> environment   = { PreviewEnvironment::Sky, "Environment", this };
    Variable<float>              sun_azimuth   = { 45.0f, "Sun azimuth", this, 0.0f, 360.0f };
    Variable<float>              sun_elevation = { 35.3f, "Sun elevation", this, 5.0f, 90.0f };
    // Studio only: scales every light but the key.
    Variable<float>              studio_fill   = { 1.0f, "Studio fill", this, 0.0f, 4.0f };
    Variable<PreviewLobes>       lobes         = { PreviewLobes::All, "Lobes", this };
};

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

        // Groups meshes_renderer's Properties-tree entry under a name distinct
        // from the other mesh_renderer instances elsewhere in the app (main
        // view x2, SceneTextureRenderer) -- AssetRenderer doesn't derive
        // VariableContext itself, hence create() rather than a direct member.
        std::unique_ptr<VariableContext> preview_context = VariableContext::create(L"Asset Preview");

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

        // Previews are drawn on the render thread (AssetManager::tact), which must not wait. A preview
        // whose materials are still compiling calls defer_preview() instead of drawing; the caller asks
        // take_deferred() afterwards and tries again on a later frame.
        static void defer_preview();
        static bool take_deferred();

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

        // See AssetRenderer::preview_context above -- same reasoning.
        std::unique_ptr<VariableContext> preview_context = VariableContext::create(L"Scene Texture Preview");

        MeshAssetInstance::ptr mesh_plane;
        MeshAssetInstance::ptr material_tester; // test mesh for material previews
        // One instance per PreviewSettings::shape other than the material tester, made on first use.
        std::map<PreviewShape, MeshAssetInstance::ptr> shape_instances;

        // Owned here so each preview has its own; attached under preview_context.
        std::unique_ptr<PreviewSettings> settings;

        // The two renders PreviewLobes::LobeDifference subtracts.
        HAL::Texture::ptr lobe_with, lobe_without;

        Scene::ptr scene;
        std::mutex lock;

        MeshAssetInstance::ptr get_shape_instance();
        void render_scene(Scene::ptr scene, HAL::Texture::ptr result, bool hide_lobe);
        void subtract_lobes(HAL::Texture::ptr with, HAL::Texture::ptr without, HAL::Texture::ptr result);

        // Orbit camera around the auto-fit center: yaw/pitch + zoom distance.
        vec2  m_orbit = { 0.785398f, 0.35f }; // ~pi/4 yaw, slight pitch
        float m_zoom  = 1.0f;

    public:
        SceneTextureRenderer();

        PreviewSettings& get_settings() { return *settings; }

        camera& get_camera() { return cam; }

        // Drag to orbit, wheel to zoom (forwarded from the preview widget).
        void orbit(vec2 delta);
        void zoom(float amount);

        // Current orbit (yaw, pitch) / zoom, so a caller can copy this
        // renderer's view onto something else (e.g. universal_material::
        // preview_orbit/preview_zoom -- see asset_preview_content).
        vec2  get_orbit() const { return m_orbit; }
        float get_zoom() const { return m_zoom; }

        void draw(Scene::ptr scene, HAL::Texture::ptr result);
        void draw(scene_object::ptr obj, HAL::Texture::ptr result);
        void draw(MaterialAsset::ptr m, HAL::Texture::ptr result);
};