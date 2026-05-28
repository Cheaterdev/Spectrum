module HAL:Fence;
import Core;
import :Types;

namespace HAL
{

	FenceWaiter::operator bool() { return !!fence; }

	void FenceWaiter::wait() const
	{
		if (fence)
		{
			fence->wait(value);
		}
	}

	bool FenceWaiter::is_completed() const
	{
		if (fence)
		{
			return fence->get_completed_value() >= value;
		}
		return true;
	}

	void FenceWaiter::combine(const FenceWaiter& other)
	{
		if (!other.fence) return;

		if (fence)
			ASSERT(fence == other.fence);
		else
		{
			fence = other.fence;
			invoker = other.invoker;
		}
		if (fence)
			value = std::max(value, other.value);
	}

}
