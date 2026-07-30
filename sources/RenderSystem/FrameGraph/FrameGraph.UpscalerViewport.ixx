export module FrameGraph:UpscalerViewport;

import Core;

export namespace FrameGraph
{
	// Stable numeric "viewport" identity for upscaler SDKs that need one
	// (NVIDIA Streamline's ViewportHandle; AMD FidelityFX/Intel XeSS have
	// their own equivalent). One Graph owns one, reclaimed on destruction.
	class UpscalerViewport
	{
		uint32_t handle;

	public:
		UpscalerViewport();
		~UpscalerViewport();

		UpscalerViewport(const UpscalerViewport&) = delete;
		UpscalerViewport& operator=(const UpscalerViewport&) = delete;

		operator uint32_t() const { return handle; }
	};
}
