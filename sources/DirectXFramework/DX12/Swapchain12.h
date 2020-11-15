#pragma once

namespace DX12
{

	struct hwnd_provider
	{
		virtual	HWND get_hwnd() const = 0;
	};

	struct swap_chain_desc
	{
		int max_fps;
		DXGI_FORMAT format;
		hwnd_provider* window;
		hwnd_provider* fullscreen;
		bool stereo;
	};


	class Texture;

	class SwapChain
	{
		void on_change();
		UINT64 last_time;
		ComPtr<IDXGISwapChain3> m_swapChain;

		struct Frame
		{
			std::shared_ptr<Texture> m_renderTarget;
			FenceWaiter fence_event;
		};

		std::vector<Frame> frames;
		DescriptorHeap::ptr heap;
		DXGI_SWAP_CHAIN_DESC desc;

	public:
		using ptr = std::shared_ptr<SwapChain>;

		UINT m_frameIndex;
		SwapChain(ComPtr<IDXGISwapChain3> sc, swap_chain_desc  c_desc);
		void present(FenceWaiter event_time);

		void start_next();
		std::shared_ptr<Texture> get_current_frame();
		void resize(ivec2 size);

	};
}