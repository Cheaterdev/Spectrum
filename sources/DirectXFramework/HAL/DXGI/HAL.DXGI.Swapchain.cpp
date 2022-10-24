module Graphics;
import :Queue;
import :Device;

import HAL;

import d3d12;
#undef THIS
namespace HAL
{
	SwapChain::SwapChain(SwapChain::native_container sc, swap_chain_desc c_desc)
	{
		sc->GetDesc(&desc);
		m_swapChain = sc;

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
			frames[n].m_renderTarget.reset(new Texture(render_target, ResourceState::PRESENT));
			frames[n].m_renderTarget->set_name(std::string("swap_chain_") + std::to_string(n));
		}
	}

	void SwapChain::resize(ivec2 size)
	{
		if (size.x < 64) size.x = 64;
		if (size.y < 64) size.y = 64;

		if (size.x == desc.BufferDesc.Width)
			if (size.y == desc.BufferDesc.Height)
				return;

		auto& q = Graphics::Device::get().get_queue(CommandListType::DIRECT);

		q->signal_and_wait();


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