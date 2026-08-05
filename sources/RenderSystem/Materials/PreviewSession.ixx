export module Graphics:Materials.PreviewSession;

import :Materials.UniversalMaterial;
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

		// Node preview mode: 2D is the original flat per-node dispatch (each
		// node's raw value coerced straight into its slice); 3D evaluates the
		// same graph over an analytic unit sphere instead of a flat UV quad
		// (real pos/normal/uv per pixel) and lights the captured value with
		// them, so e.g. triplanar/normal/world-pos-driven nodes preview
		// correctly and everything reads as an actual shaded surface instead
		// of a flat swatch. Switching it is a PSO macro (PREVIEW_3D), so it
		// forces the same rebuild path as a structural graph change -- see
		// rebuild_pso().
		bool want_3d = false;

		HAL::Texture::ptr results;
		std::vector<HAL::Texture2DView> slice_views;
		PSOS::MaterialPreview::ptr pso;

		void rebuild_pso();
		void dispatch();

	public:
		static constexpr int preview_resolution = 64;

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
