export module HAL:Fence;
import :Device;

import d3d12;
import windows;
import Profiling;

export namespace HAL
{
	class Event
	{
		friend class Fence;
		HANDLE m_fenceEvent;
	public:
		Event();
		~Event();

		void wait();
	};

	class Fence
	{

	public:
//		friend class Queue;
		D3D::Fence m_fence;
	public:
		using CounterType = UINT64;

		Fence(Device& device);

		void signal(CounterType value);
		CounterType get_completed_value() const;
		void wait(CounterType value) const;
	};


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
		device.native_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	}

	void Fence::signal(CounterType value)
	{
		m_fence->Signal(value);
	}

	Fence::CounterType Fence::get_completed_value() const
	{
		return m_fence->GetCompletedValue();
	}

	void Fence::wait(Fence::CounterType value) const
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



