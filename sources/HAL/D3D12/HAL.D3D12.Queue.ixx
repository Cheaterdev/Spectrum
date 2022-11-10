export module HAL:API.Queue;
import :Types;
import :Utils;
import :Fence;
import :CommandList;

export namespace HAL
{
	namespace API
	{
		class Queue
		{
		protected:
			D3D::CommandQueue native;

			void execute(CommandList* list);
			void execute(TransitionCommandList* list);
			void signal(Fence& fence, Fence::CounterType value);
			void gpu_wait(HAL::FenceWaiter waiter);


			void construct(HAL::CommandListType type, Device* device);
		public:
			virtual ~Queue() = default;

			D3D::CommandQueue get_native();
		};
	}

}