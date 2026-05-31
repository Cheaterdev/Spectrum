module HAL:TiledMemoryManager;
import Core;
import HAL;

// Vulkan stub for TiledResourceManager::init_tilings().
// Sparse/tiled resources (VK_KHR_sparse) are a post-Phase-0 feature.
// The function intentionally does nothing so that resources that query tiling
// info simply report no tiles, disabling the tiled-resource path.

namespace HAL
{
    void TiledResourceManager::init_tilings()
    {
        // Not implemented for Vulkan (Phase 4+ / sparse resource support).
    }
}
