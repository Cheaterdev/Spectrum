export module HAL:SwapChain;

import Core;
import :Types;
import :Format;
import :Texture;
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
			std::shared_ptr<Texture> m_renderTarget;
			FenceWaiter fence_event;
		};

		std::vector<Frame> frames;
		Device& device;
	public:
		using ptr = std::shared_ptr<SwapChain>;

		SwapChain(Device& device, swap_chain_desc c_desc);
		void present(FenceWaiter event_time);
		UINT m_frameIndex;

		void start_next();
		std::shared_ptr<Texture> get_current_frame();
		std::shared_ptr<Texture> get_prev_frame();
		void resize(ivec2 size);
		void wait_for_free();
	};
}