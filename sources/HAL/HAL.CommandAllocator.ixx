export module HAL:CommandAllocator;

import Core;
import :Types;

import :API.CommandAllocator;

export{

	namespace HAL
	{

		class CommandAllocator : public API::CommandAllocator
		{
			CommandListType type;
		public:
			CommandAllocator(CommandListType type);
			void reset();
			CommandListType get_type() const { return type; }
		};

	}

}