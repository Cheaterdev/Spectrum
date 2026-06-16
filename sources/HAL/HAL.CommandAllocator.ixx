export module HAL:CommandAllocator;

import Core;
import :Types;

import :API.CommandAllocator;

export{

	namespace HAL
	{

		class CommandAllocator : public API::CommandAllocator
		{
			Device& device;
			CommandListType type;
		public:
			CommandAllocator(Device& device, CommandListType type);
			~CommandAllocator();
			void reset();
			CommandListType get_type() const;
		};

	}

}