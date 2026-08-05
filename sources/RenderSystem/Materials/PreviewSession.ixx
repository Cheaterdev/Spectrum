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

		static void open(::FlowGraph::graph* g);
		static void close(::FlowGraph::graph* g);
		static MaterialPreviewSession* find(::FlowGraph::graph* g);
	};
}
