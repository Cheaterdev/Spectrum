module FrameGraph:UpscalerViewport;

import Core;

namespace FrameGraph
{
	namespace
	{
		IdGenerator<Thread::Lockable> g_ids;
	}

	UpscalerViewport::UpscalerViewport()
	{
		handle = uint32_t(g_ids.get());
	}

	UpscalerViewport::~UpscalerViewport()
	{
		g_ids.put(int(handle));
	}
}
