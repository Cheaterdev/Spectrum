export module HAL:SwapChain;

import Core;
import :Types;
import :Format;
import :Resource.Texture;
import :Fence;
import :API.SwapChain;
import :Device;

export namespace HAL
{
	class SwapChain :public API::SwapChain
	{
		friend class API::SwapChain;
		void on_change();
		UINT64 last_time;

		struct Frame
		{
			std::shared_ptr<TextureResource> m_renderTarget;
			FenceWaiter fence_event;
		};

		std::vector<Frame> frames;
		Device& device;
		void* native_window = nullptr;
		DisplayInfo display_info;

		// The backbuffers are allocated at `capacity` (>= the window) and the visible
		// region is selected with SetSourceSize, so shrinking or growing the window
		// within the capacity needs no ResizeBuffers and no device drain.
		ivec2 logical_size = { 0, 0 };
		ivec2 capacity = { 0, 0 };
		bool oversized = false;
		uint64_t last_resize_tick = 0;
		void reallocate(ivec2 new_capacity);
	public:
		using ptr = std::shared_ptr<SwapChain>;

		SwapChain(Device& device, swap_chain_desc c_desc);
		void present();
		UINT m_frameIndex;
		FenceWaiter get_fence();
				 
		const std::shared_ptr<TextureResource>& get_current_frame();
		const std::shared_ptr<TextureResource>& get_prev_frame();
		void resize(ivec2 size);
		// Shrinks an oversized allocation back to the window size once resizing has stopped.
		void trim_if_idle();
		// Visible size; the backbuffer textures may be larger.
		ivec2 get_size() const { return logical_size; }
		// Present without waiting for vblank: used for frames made for a new window size, so live resize isn't paced at 2 vsyncs per size message.
		bool present_immediate = false;
		void wait_for_free();

		// Re-queries the display under the window (HDR on/off, luminance,
		// SDR white level). Cheap enough to call a few times a second; picks up
		// the window moving to another monitor or HDR being toggled in Windows.
		void refresh_display_info();
		const DisplayInfo& get_display_info() const { return display_info; }
	};
}