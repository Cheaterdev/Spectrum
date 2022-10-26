export module HAL:API.SwapChain;

import Math;
import d3d12;
import :Types;

export
{

	namespace HAL
	{

	
		
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