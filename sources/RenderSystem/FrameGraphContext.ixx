export module Graphics:FrameGraphContext;

import :Camera;
import :MeshRenderer;
import :Scene;

export
{
										// Viewport dimensions set once per frame before graph setup.
	// Lives here (not in Graphics:FrameGraphContext) so it is visible
	// inside FrameGraph:Passes and pass_defaults.h without creating
	// circular module dependencies.
	struct ViewportInfo
	{
		ivec2 frame_size;
		ivec2 upscale_size;
	};

	struct TimeInfo
	{
		float time;
		float totalTime = 0;
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