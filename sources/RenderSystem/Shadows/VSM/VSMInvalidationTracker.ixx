export module Graphics:VSMInvalidationTracker;

import Core;
import :Scene;

export
{
	// Phase 2 caching: per-level, per-page dirty tracking. A scene event
	// (add/remove/move/material change) maps the touched object's world
	// AABB to a (level, page) bit range via VSM's own clipmap math (VSM
	// owns that mapping and calls mark_pages() here -- this class just
	// stores the registration and the resulting masks, kept dumb on
	// purpose so it doesn't need to know about VSMClipmap/light direction
	// at all).
	//
	// Inherits Events::prop_handler purely because register_handler
	// requires it (matching mesh_renderer/VoxelGI, which do the same).
	class VSMInvalidationTracker : public Events::prop_handler
	{
		std::mutex m;
		static constexpr int MaxLevels = 8;
		std::array<uint32_t, MaxLevels> dirty_masks{};

	public:
		VSMInvalidationTracker()
		{
			// Every page starts dirty: forces the initial fill (matches
			// level_initialized/light_initialized also forcing a full
			// render the first time each level runs).
			dirty_masks.fill(0xFFFFFFFFu);
		}

		void attach(std::shared_ptr<Scene> scene, std::function<void(scene_object*)> on_change)
		{
			scene->on_element_add.register_handler(this, on_change);
			scene->on_element_remove.register_handler(this, on_change);
			scene->on_moved.register_handler(this, on_change);
			scene->on_changed.register_handler(this, on_change);
		}

		void mark_pages(int level, uint32_t mask)
		{
			std::lock_guard<std::mutex> g(m);
			dirty_masks[level] |= mask;
		}

		void mark_level_full(int level)
		{
			std::lock_guard<std::mutex> g(m);
			dirty_masks[level] = 0xFFFFFFFFu;
		}

		// Read-and-clear: safe to call once per level per frame (each level
		// owns its own mask, unlike the single shared flag this replaced --
		// no cross-level "steals the flag" ordering concern here).
		uint32_t take_dirty(int level)
		{
			std::lock_guard<std::mutex> g(m);
			uint32_t v = dirty_masks[level];
			dirty_masks[level] = 0;
			return v;
		}
	};
}
