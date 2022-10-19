export module HAL:Removeme;

import stl.memory;

export namespace HAL
{
    class CommandList
    {
    	public:
        CommandList() = default;
        virtual ~CommandList() = default;
	    
    };
    class TransitionCommandList
    {
    	public:
            TransitionCommandList() = default;
        virtual ~TransitionCommandList() = default;
	    
    };

    class Transitions
    {
    	public:
            Transitions() = default;
        virtual ~Transitions() = default;
	    
    };

    using CommandListPtr = std::shared_ptr<CommandList>;
	using TransitionCommandListPtr = std::shared_ptr<TransitionCommandList>;
	using TransitionsPtr = std::shared_ptr<Transitions>;

}
