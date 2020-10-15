#include "pch.h"


namespace DX12
{

	SwapChain::SwapChain(ComPtr<IDXGISwapChain3> sc, swap_chain_desc c_desc)
	{
		sc->GetDesc(&desc);
		m_swapChain = sc;
		heap.reset(new DescriptorHeap(desc.BufferCount, DescriptorHeapType::RTV));
		frames.resize(desc.BufferCount);
		on_change();
	}

	void  SwapChain::on_change()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		heap->reset();

		// Create a RTV for each frame.
		for (UINT n = 0; n < frames.size(); n++)
		{
			ComPtr<ID3D12Resource> render_target;
			m_swapChain->GetBuffer(n, IID_PPV_ARGS(&render_target));
			auto handle = heap->create_table(1);

			frames[n].m_renderTarget.reset(new Texture(render_target, handle, Render::ResourceState::PRESENT));
			frames[n].m_renderTarget->set_name(std::string("swap_chain_") + std::to_string(n));
		//	frames[n].m_renderTarget->assume_gpu_state(Render::ResourceState::PRESENT);

		//	frames[n].m_renderTarget->debug = true;


			D3D12_RENDER_TARGET_VIEW_DESC desc = {};
			desc.Format = to_srv(frames[n].m_renderTarget->get_desc().Format);
			desc.ViewDimension = D3D12_RTV_DIMENSION::D3D12_RTV_DIMENSION_TEXTURE2D;
			desc.Texture2D.MipSlice = 0;
			desc.Texture2D.PlaneSlice = 0;
	



			Device::get().create_rtv(handle[0], frames[n].m_renderTarget.get(), desc);



		}
	}

		void SwapChain::present(UINT64 event_time)
		{
		//	Log::get() << "present" << Log::endl;
			//assert(frames[m_frameIndex].m_renderTarget->get_gpu_state() == 0);

			frames[m_frameIndex].fence_event = event_time;
			last_time = event_time;
			m_swapChain->Present(0, 0);
		}
		

	void SwapChain::start_next()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	
			Device::get().get_queue(CommandListType::DIRECT)->wait(frames[m_frameIndex].fence_event);


			if (GetAsyncKeyState(VK_F8))
		for (auto &f : frames)						Device::get().get_queue(CommandListType::DIRECT)->wait(f.fence_event);
				

	}


	Texture::ptr SwapChain::get_current_frame()
	{
		return frames[m_frameIndex].m_renderTarget;
	}



	void SwapChain::resize(ivec2 size)
	{
		auto & q = Device::get().get_queue(CommandListType::DIRECT);

		q->wait(q->signal());

		//	frames[0].m_renderTarget.
		for (auto && f : frames)
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