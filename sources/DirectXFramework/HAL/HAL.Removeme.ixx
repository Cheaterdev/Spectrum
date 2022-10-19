export module HAL:Removeme;

import stl.memory;

export namespace HAL
{
    class CommandList
    {public:
        CommandList() = default;
        virtual ~CommandList() = default;
	    
    };
    using CommandListPtr = std::shared_ptr<CommandList>;
}
