export module Graphics:FrameGraphContext;

import :Camera;
import :MeshRenderer;
import :Scene;
import HAL;
import Core;

export
{
										// Viewport dimensions set once per frame before graph setup.
	// Lives here (not in Graphics:FrameGraphContext) so it is visible
	// inside FrameGraph:Passes and pass_defaults.h without creating
	// circular module dependencies.
	//
	// frame_size/upscale_size come from Table::ViewportContext (FrameData.sig)
	// -- the SIG-declared schema [Size=...] annotations resolve against --
	// rather than being redeclared here, so the two can never drift apart.
	// Any field ViewportInfo needs that FrameGraph/SIG has no reason to know
	// about goes directly on this derived type instead.
	struct ViewportInfo : Table::ViewportContext
	{
	};

	// get_context<Table::ViewportContext>() (what SIG-generated [Size=...]
	// resolution will call) finds this same live ViewportInfo instance
	// instead of default-constructing a disconnected, always-zeroed one.
	template<> struct ContextTypeFor<Table::ViewportContext> { using type = ViewportInfo; };

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