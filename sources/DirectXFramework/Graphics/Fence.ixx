export module Graphics:Fence;
import HAL;

export namespace Graphics
{
	using Fence = HAL::Fence;

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