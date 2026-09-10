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
	// SIG-declared Table::ViewportContext) lives in FrameGraph:ViewportContext,
	// not here -- the generated create_always()/need_always() calls resolving
	// [Size=ViewportContext::frame_size] are compiled as part of the FrameGraph
	// module and need the ContextTypeFor redirect visible at that point, which
	// a declaration up here in Graphics could never provide. Re-exported
	// transitively via `import FrameGraph;` above.

	struct TimeInfo
	{
		float time;
		float totalTime = 0;
	};

	struct SkyInfo
	{
		float3 sunDir;
		// Set by CubeSky::setup() (Sky.cpp) whenever sunDir moved enough to
		// warrant a re-bake, read by CubeMapDownsample/CubeMapEnviromentProcessor's
		// own setup() to decide whether they need to re-run this frame. A
		// plain context field, not FrameGraph's resource-level is_changed() --
		// that one required create() and changed() to run in the same
		// function (create_always() runs after setup_func, so a downstream
		// pass's setup() -- same phase, same frame -- could never observe a
		// flag set later, in render()). This field has no such ordering
		// constraint: CubeSky's setup() runs before the passes that read it,
		// in the same Graph::setup() pass loop, every frame.
		bool sky_changed = false;
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