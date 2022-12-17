module HAL:SwapChain;



namespace HAL
{
	void SwapChain::wait_for_free()
	{
		frames[m_frameIndex].fence_event.wait();
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