module;
#include <comdef.h>

module HAL:SwapChain;
import:Device;
import HAL;


import d3d12;

namespace HAL
{
	SwapChain::SwapChain(Device& device, swap_chain_desc c_desc) :device(device)
	{

		RECT r;
		GetClientRect(c_desc.window->get_hwnd(), &r);
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = r.right - r.left;
		swapChainDesc.Height = r.bottom - r.top;
		swapChainDesc.Format = ::to_native(c_desc.format);
		swapChainDesc.Stereo = c_desc.stereo && HAL::Adapters::get().get_factory()->IsWindowedStereoEnabled();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI::USAGE_RENDER_TARGET_OUTPUT | DXGI::USAGE_BACK_BUFFER;
		swapChainDesc.BufferCount = 2 + static_cast<int>(swapChainDesc.Stereo);
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		//swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		
		ComPtr<IDXGISwapChain1> swapChain;
		HRESULT res = HAL::Adapters::get().get_factory()->CreateSwapChainForHwnd(
			device.get_queue(CommandListType::DIRECT)->get_native().Get(),
			c_desc.window->get_hwnd(),
			&swapChainDesc, nullptr, nullptr, &swapChain);

		swapChain.As(&m_swapChain);

		// Manual hooking's substitute for the interposer's automatic
		// IDXGISwapChain::Present hook — see [[project-streamline-dlss-integration]].
		if (nvidia::Streamline::get().available())
		{
			IDXGISwapChain3* raw = m_swapChain.Detach();
			void* raw_void = raw;
			if (nvidia::Streamline::get().upgrade_interface(&raw_void))
				raw = static_cast<IDXGISwapChain3*>(raw_void);
			m_swapChain.Attach(raw);
		}

		m_swapChain->GetDesc(&desc);

		frames.resize(desc.BufferCount);
		on_change();
		  
	//	swapchain_waiter = m_swapChain->GetFrameLatencyWaitableObject();

		//DWORD result = WaitForSingleObjectEx(
		//	swapchain_waiter,
		//	1000, // 1 second timeout (shouldn't ever occur)
		//	true
		//);
	}

	void SwapChain::present()
	{
		//uint index = m_frameIndex;

		auto native = m_swapChain;
		device.get_queue(CommandListType::DIRECT)->run([native, this](){
			PROFILE(L"Present")
					native->Present(1, 0);
			});
	
		frames[m_frameIndex].fence_event = device.get_queue(CommandListType::DIRECT)->signal();

		m_frameIndex = 1-m_frameIndex;
	/*	DWORD result = WaitForSingleObjectEx(
			swapchain_waiter,
			1000, // 1 second timeout (shouldn't ever occur)
			true
		);		 */
	}

	void  SwapChain::on_change()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		for (UINT n = 0; n < frames.size(); n++)
		{
			D3D::Resource  render_target;
			m_swapChain->GetBuffer(n, IID_PPV_ARGS(&render_target));
			frames[n].m_renderTarget.reset(new TextureResource(device, API::NativeImportHandle{render_target}, TextureLayout::PRESENT));
			frames[n].m_renderTarget->set_name(std::string("swap_chain_") + std::to_string(n));
			
		}

		frames[0].m_renderTarget->debug=true;

			m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	void SwapChain::resize(ivec2 size)
	{
		if (size.x < 64) size.x = 64;
		if (size.y < 64) size.y = 64;

		if (size.x == desc.BufferDesc.Width)
			if (size.y == desc.BufferDesc.Height)
				return;

		//	device.get_queue(CommandListType::DIRECT)->sync();

		{
			for (auto type : magic_enum::enum_values<CommandListType>())
				device.get_queue(type)->signal_and_wait();
		}

		for (auto&& f : frames)
			f.m_renderTarget = nullptr;

		desc.BufferDesc.Width = size.x;
		desc.BufferDesc.Height = size.y;
		m_swapChain->ResizeBuffers(desc.BufferCount,
			desc.BufferDesc.Width,
			desc.BufferDesc.Height,
			desc.BufferDesc.Format,
			desc.Flags);
		on_change();
	}
}