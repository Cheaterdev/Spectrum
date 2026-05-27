module Core:Exceptions;

namespace Exceptions
{

	Exception::Exception(std::string_view wtf) : std::exception((std::string(wtf) + "\n At:" /* + boost::stacktrace::to_string(stack)*/).c_str())
	{
	}

	stack_trace Exception::get_stack_trace()
	{
		return stack;
	}

	stack_trace get_stack_trace()
	{
		return 1;//std::stacktrace::current();
	}

}
