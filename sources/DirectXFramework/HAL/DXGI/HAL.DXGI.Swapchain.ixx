export module HAL:API.SwapChain;

import Math;
import d3d12;
import :Types;

export
{

	namespace HAL
	{

		struct hwnd_provider
		{
			virtual	HWND get_hwnd() const = 0;
		};

		struct swap_chain_desc
		{
			int max_fps;
			Format format;
			hwnd_provider* window;
			hwnd_provider* fullscreen;
			bool stereo;
		};

		
		namespace API
		{
			class SwapChain
			{
			protected:
				ComPtr<IDXGISwapChain3> m_swapChain;
				DXGI_SWAP_CHAIN_DESC desc;

				using native_container = ComPtr<IDXGISwapChain3>;
			public:
				
				virtual ~SwapChain() = default;
			};
		}

		
	}
}