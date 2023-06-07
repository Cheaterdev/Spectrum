module HAL:SwapChain;
import :Queue;
import :Device;



namespace HAL
{
	void SwapChain::wait_for_free()
	{
		frames[m_frameIndex].fence_event.wait();

			if(GetAsyncKeyState('0'))
			{
			
									HAL::Device::get().get_queue(HAL::CommandListType::DIRECT)->signal_and_wait();
				HAL::Device::get().get_queue(HAL::CommandListType::COMPUTE)->signal_and_wait();
				HAL::Device::get().get_queue(HAL::CommandListType::COPY)->signal_and_wait();
			
			}
//for(auto &f:frames)
//{
//f.fence_event.wait();
//}
	}

	const Texture::ptr& SwapChain::get_current_frame()
	{
		return frames[m_frameIndex].m_renderTarget;
	}

	const Texture::ptr& SwapChain::get_prev_frame()
	{
		int id = m_frameIndex - 1;
		if (id == -1)id = static_cast<int>(frames.size() - 1);
		return frames[id].m_renderTarget;
	}

}