export module HAL:Removeme;

import stl.memory;
import Data;

export namespace HAL
{
    class CommandList:public SharedObject<CommandList>
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
    class FrameResources
    {
    public:
        FrameResources() = default;
        virtual ~FrameResources() = default;

    };

    using CommandListPtr = std::shared_ptr<CommandList>;
	using TransitionCommandListPtr = std::shared_ptr<TransitionCommandList>;
	using TransitionsPtr = std::shared_ptr<Transitions>;
using FrameResourcesPtr = std::shared_ptr<FrameResources>;

}
