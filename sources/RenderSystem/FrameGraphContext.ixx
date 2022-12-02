export module Graphics:FrameGraphContext;

import :Camera;
import :MeshRenderer;
import :Scene;

export
{


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
		main_renderer* renderer;
		Scene* scene;
	};

	struct CameraInfo
	{
		camera* cam;
	};

	struct ViewportInfo
	{
		ivec2 frame_size;
		ivec2 upscale_size;
	};
}