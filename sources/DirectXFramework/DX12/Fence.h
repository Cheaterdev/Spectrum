#pragma once
namespace DX12
{
	class Device;

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
		friend class Queue;
		ComPtr<ID3D12Fence> m_fence;

	public:
		
		Fence(Device & device);

		void signal(UINT64 value);
		UINT64 get_completed_value();
		void wait_for(Event & e, UINT64 value);

	};


	struct FenceWaiter
	{
		const Fence& fence;
		const UINT64 value;
	};
}