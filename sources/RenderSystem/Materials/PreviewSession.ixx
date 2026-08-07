export module Graphics:Materials.PreviewSession;

import :Materials.UniversalMaterial;
import :Scene;
import :MeshAsset;
import :Camera;
import HAL;
import Core;
import GUI;

export namespace materials
{
	// Owns the GPU-resident state for a material graph's per-node live
	// previews (PSO, results array texture, per-slice SRVs) for as long as
	// its editor window is open. Constructed/destroyed by GUI::Elements::
	// FlowGraph::canvas's on_open/on_close hooks (see main.cpp) -- a
	// material never opened in the editor never pays for any of this.
	class MaterialPreviewSession
	{
		universal_material* material;

		// ~0u so a freshly-constructed session always rebuilds/redispatches
		// on its first get_slice_view() call.
		uint32_t built_source_generation = ~0u;
		uint32_t dispatched_generation = ~0u;
		bool built_3d = false;

		// Last orbit/zoom (see universal_material::preview_orbit/preview_zoom)
		// a 3D dispatch actually used -- so get_slice_view() can redispatch
		// when the user orbits the material's full preview even though
		// preview_generation itself hasn't moved (no material data changed).
		vec2  dispatched_orbit = { -1, -1 };
		float dispatched_zoom  = -1;

		// Node preview mode: 2D is the original flat per-node compute dispatch
		// (each node's raw value coerced straight into its slice). 3D instead
		// draws the real material_tester mesh with a direct (non-indirect)
		// dispatch_mesh -- see dispatch() -- so COMPILED_FUNC (and every
		// node's capture write) gets real interpolated position/normal/UV
		// per pixel, and the captured value gets lit with a simple fixed
		// light instead of shown flat. Switching it forces a PSO rebuild
		// (different PSO type entirely) -- see rebuild_pso().
		bool want_3d = false;

		HAL::Texture::ptr results;
		std::vector<HAL::Texture2DView> slice_views;
		PSOS::MaterialPreview::ptr pso;
		PSOS::MaterialPreview3D::ptr pso3d;

		// 3D mode only: real hidden-surface removal for the rasterized mesh
		// (there's no other visibility resolution -- no rtv, UAV-only PS).
		// Always preview_resolution x preview_resolution, so built once and
		// reused for the session's lifetime, unlike results (which resizes
		// with node/slot count).
		HAL::Texture::ptr preview_depth;

		// 3D mode's private mesh/scene/camera -- built lazily on first 3D
		// dispatch, kept for the session's lifetime. Never enters any
		// FrameGraph; dispatch() issues its own direct command list, mirroring
		// how the 2D path already does an ad hoc immediate dispatch.
		Scene::ptr           preview_scene;
		MeshAssetInstance::ptr preview_mesh;
		camera                preview_cam;
		void ensure_3d_scene();

		void rebuild_pso();
		void dispatch();

	public:
		// Matches node_preview_thumbnail's display size (main.cpp) 1:1 --
		// anything smaller gets upscaled/blurred by the GUI.
		static constexpr int preview_resolution = 128;

		MaterialPreviewSession(universal_material* material);

		// Rebuilds the PSO/redispatches only if the material has changed
		// since our last call, then returns the requested slot's slice view
		// (empty if out of range or nothing captured for that node).
		HAL::Texture2DView get_slice_view(int slot);

		bool is_3d() const { return want_3d; }
		void set_3d(bool value) { want_3d = value; }

		static void open(::FlowGraph::graph* g);
		static void close(::FlowGraph::graph* g);
		static MaterialPreviewSession* find(::FlowGraph::graph* g);
	};

	// Filled by the app layer (main.cpp) -- builds the "Material Settings"
	// side panel widget for a material graph's editor dock. Materials can't
	// reference that class directly (it's app-layer GUI, same reason as
	// MaterialGraph::create_preview_hook).
	inline std::function<GUI::base::ptr(universal_material*, ::FlowGraph::graph*)> create_settings_panel_hook;

	// Builds the full material-graph editor content for g: the canvas, plus
	// -- if g is an actual MaterialGraph with a live preview_material and
	// create_settings_panel_hook is set -- a "Material Settings" side panel,
	// both inside one self-contained dock. Mirrors what canvas::on_open used
	// to build after the fact once a tab_button for g showed up in the
	// manager's registry, but does it synchronously and up front instead --
	// that registry lookup broke once callers stopped pre-building their own
	// tab_button (see GUI::Elements::FlowGraph::manager::create_canvas/
	// register_tab), since register_tab() now runs after the tab (and thus
	// this canvas) is already attached.
	GUI::base::ptr open_material_editor(::FlowGraph::graph::ptr g);
}
