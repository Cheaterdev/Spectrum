export module Graphics:Fence;

import d3d12;
import windows;
using namespace Microsoft::WRL;
export
{


namespace Graphics
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
		UINT64 get_completed_value() const;


		void wait(UINT64 value) const;

	};


	struct FenceWaiter
	{
		Fence* fence = nullptr;
		UINT64 value;

		operator bool()
		{
			return !!fence;
		}
		void wait() const
		{
			if (fence)
			{
				fence->wait(value);
			}
		}
	};
}
}