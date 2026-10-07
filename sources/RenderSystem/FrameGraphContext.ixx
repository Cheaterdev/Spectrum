export module Graphics:FrameGraphContext;

import :Camera;
import :MeshRenderer;
import :Scene;
export import FrameGraph;
import HAL;
import Core;

export
{
	// ViewportInfo itself (frame_size/upscale_size, inherited from the
	// SIG-declared Table::Frame::ViewportContext) lives in FrameGraph:ViewportContext,
	// not here -- the generated create_always()/need_always() calls resolving
	// [Size=ViewportContext::frame_size] are compiled as part of the FrameGraph
	// module and need the ContextTypeFor redirect visible at that point, which
	// a declaration up here in Graphics could never provide. Re-exported
	// transitively via `import FrameGraph;` above.

	struct TimeInfo
	{
		float time;
		float totalTime = 0;
		// This graph's frame number, the one clock for everything per-frame:
		// noise sequences (blue noise, jitter), staggered updates and temporal
		// history, whose owners reset when it isn't consecutive (a new graph).
		// Not the device's frame counter, which every graph on the device
		// advances, nor a counter of the owner's own, which outlives the graph.
		uint64 frame = 0;
	};

	struct SkyInfo
	{
		float3 sunDir;
	};
	struct SceneInfo
	{
		main_renderer::ptr renderer;
		Scene::ptr         scene;
	};

	struct CameraInfo
	{
		camera* cam;
	};

}