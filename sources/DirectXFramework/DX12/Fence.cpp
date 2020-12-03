#include "pch.h"

namespace DX12
{

	Event::Event()
	{
		m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	}

	Event::~Event()
	{
		CloseHandle(m_fenceEvent);
	}

	void Event::wait()
	{
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	Fence::Fence(Device& device)
	{
		device.get_native_device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	}

	void Fence::signal(UINT64 value)
	{
		m_fence->Signal(value);
	}

	UINT64 Fence::get_completed_value() const
	{
		return m_fence->GetCompletedValue();
	}

	void Fence::wait(UINT64 value) const
	{
		static thread_local Event e;
	//	UINT64 compl = m_fence->GetCompletedValue();

		if (m_fence->GetCompletedValue() < value)
		{
			PROFILE(L"FenceWait");
			m_fence->SetEventOnCompletion(value, e.m_fenceEvent);
			e.wait();
		}
	}
}
