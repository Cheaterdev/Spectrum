export module HAL:Fence;
import :API.Device;
import :API.Fence;

export namespace HAL
{
	class Event : public API::Event
	{
	public:
		Event();
		~Event();

		void wait();
	};

	class Fence : public API::Fence
	{

	public:
		using CounterType = API::Fence::CounterType;
		Fence(Device& device);

		void signal(CounterType value);
		CounterType get_completed_value() const;
		void wait(CounterType value) const;
	};

	struct FenceWaiter
	{
		Fence* fence = nullptr;
		Fence::CounterType value;
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



