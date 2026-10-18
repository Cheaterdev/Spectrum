module Core:Exceptions;

namespace Exceptions
{

	Exception::Exception(std::string_view wtf) : message(std::string(wtf) + "\n At:")
	{
	}

	stack_trace Exception::get_stack_trace()
	{
		return stack;
	}

	stack_trace get_stack_trace()
	{
#if __has_include(<stacktrace>)
		return std::stacktrace::current();
#else
		return {};
#endif
	}

}
