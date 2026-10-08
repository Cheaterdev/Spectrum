module Core:Exceptions;

namespace Exceptions
{

#ifdef _WIN32
	Exception::Exception(std::string_view wtf) : std::exception((std::string(wtf) + "\n At:").c_str())
#else
	Exception::Exception(std::string_view wtf) : message(std::string(wtf) + "\n At:")
#endif
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
