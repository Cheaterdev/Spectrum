module HAL:SwapChain;
import:Device;
import HAL;

import <HAL.h>;
import <d3d12/d3d12_includes.h>;
#undef THIS
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
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2 + static_cast<int>(swapChainDesc.Stereo);
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		ComPtr<IDXGISwapChain1> swapChain;
		HRESULT res = HAL::Adapters::get().get_factory()->CreateSwapChainForHwnd(
			device.get_queue(CommandListType::DIRECT)->get_native().Get(),
			c_desc.window->get_hwnd(),
			&swapChainDesc, nullptr, nullptr, &swapChain);

		swapChain.As(&m_swapChain);

		m_swapChain->GetDesc(&desc);

		frames.resize(desc.BufferCount);
		on_change();
	}

	void SwapChain::present(FenceWaiter event_time)
	{
		frames[m_frameIndex].fence_event = event_time;
		m_swapChain->Present(0, 0);
	}

	void SwapChain::start_next()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	void  SwapChain::on_change()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		for (UINT n = 0; n < frames.size(); n++)
		{
			D3D::Resource  render_target;
			m_swapChain->GetBuffer(n, IID_PPV_ARGS(&render_target));
			frames[n].m_renderTarget.reset(new Texture(render_target, TextureLayout::PRESENT));
			frames[n].m_renderTarget->resource->set_name(std::string("swap_chain_") + std::to_string(n));
		}
	}

	void SwapChain::resize(ivec2 size)
	{
		if (size.x < 64) size.x = 64;
		if (size.y < 64) size.y = 64;

		if (size.x == desc.BufferDesc.Width)
			if (size.y == desc.BufferDesc.Height)
				return;

		{
			device.get_queue(CommandListType::DIRECT)->signal_and_wait();
			device.get_queue(CommandListType::COMPUTE)->signal_and_wait();
			device.get_queue(CommandListType::COPY)->signal_and_wait();
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