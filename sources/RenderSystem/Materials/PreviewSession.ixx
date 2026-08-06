export module Graphics:Materials.PreviewSession;

import :Materials.UniversalMaterial;
import :Scene;
import :MeshAsset;
import :Camera;
import HAL;
import Core;

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
}
