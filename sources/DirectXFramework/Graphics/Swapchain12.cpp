module;

#include <string>

#include "helper.h"
module Graphics:SwapChain;
import :Texture;
import :Queue;

import HAL;
using namespace HAL;

namespace Graphics
{

	SwapChain::SwapChain(ComPtr<IDXGISwapChain3> sc, swap_chain_desc c_desc)
	{
		sc->GetDesc(&desc);
		m_swapChain = sc;
		//heap.reset(new DescriptorHeap(desc.BufferCount, DescriptorHeapType::RTV));
		frames.resize(desc.BufferCount);
		on_change();
	}

	void  SwapChain::on_change()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		//heap->reset();

		// Create a RTV for each frame.
		for (UINT n = 0; n < frames.size(); n++)
		{
			D3D::Resource  render_target;
			m_swapChain->GetBuffer(n, IID_PPV_ARGS(&render_target));

			auto hal_resource = std::make_shared<HAL::Resource>(render_target);

			frames[n].m_renderTarget.reset(new Texture(hal_resource, ResourceState::PRESENT));
			frames[n].m_renderTarget->set_name(std::string("swap_chain_") + std::to_string(n));


		}
	}

	void SwapChain::wait_for_free()
	{
		frames[m_frameIndex].fence_event.wait();

	}
	void SwapChain::present(FenceWaiter event_time)
	{

		frames[m_frameIndex].fence_event = event_time;
		m_swapChain->Present(0, 0);
	}


	void SwapChain::start_next()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();


		//		if (GetAsyncKeyState(VK_F8))
			//		for (auto& f : frames)	f.fence_event.wait();
	}


	Texture::ptr SwapChain::get_current_frame()
	{
		return frames[m_frameIndex].m_renderTarget;
	}

	Texture::ptr SwapChain::get_prev_frame()
	{
		int id = m_frameIndex - 1;
		if (id == -1)id = static_cast<int>(frames.size() - 1);
		return frames[id].m_renderTarget;
	}

	void SwapChain::resize(ivec2 size)
	{
		if (size.x < 64) size.x = 64;
		if (size.y < 64) size.y = 64;

		if (size.x == desc.BufferDesc.Width)
			if (size.y == desc.BufferDesc.Height)
				return;

		auto& q = Device::get().get_queue(CommandListType::DIRECT);

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