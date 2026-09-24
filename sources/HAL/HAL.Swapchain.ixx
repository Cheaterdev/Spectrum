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
		HWND hwnd = nullptr;
		DisplayInfo display_info;
	public:
		using ptr = std::shared_ptr<SwapChain>;

		SwapChain(Device& device, swap_chain_desc c_desc);
		void present();
		UINT m_frameIndex;
		FenceWaiter get_fence();
				 
		const std::shared_ptr<TextureResource>& get_current_frame();
		const std::shared_ptr<TextureResource>& get_prev_frame();
		void resize(ivec2 size);
		void wait_for_free();

		// Re-queries the display under the window (HDR on/off, luminance,
		// SDR white level). Cheap enough to call a few times a second; picks up
		// the window moving to another monitor or HDR being toggled in Windows.
		void refresh_display_info();
		const DisplayInfo& get_display_info() const { return display_info; }
	};
}