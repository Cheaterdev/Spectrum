export module HAL:Fence;

import :Device;
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
		Fence::CounterType value = 0;
		HAL::CommandListType invoker;
		operator bool();
		void wait() const;
		bool is_completed() const;
		void combine(const FenceWaiter& other);
	};
}



