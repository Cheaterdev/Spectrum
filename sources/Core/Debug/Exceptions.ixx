export module Core:Exceptions;

//#include <boost/stacktrace.hpp>
import stl.core;
export namespace Exceptions
{
	using stack_trace = int;// boost::stacktrace::stacktrace;

	class Exception : public std::exception
	{
		stack_trace stack;
	public:
		Exception(std::string_view wtf);
		stack_trace get_stack_trace();
	};

	stack_trace get_stack_trace();	
}


namespace Exceptions
{

	Exception::Exception(std::string_view wtf) : std::exception((std::string(wtf) + "\n At:" /* + boost::stacktrace::to_string(stack)*/).c_str())
	{
	}

	stack_trace Exception::get_stack_trace()
	{
		return 	stack;
	}


	stack_trace get_stack_trace()
	{
		return 0;// boost::stacktrace::stacktrace();
	}
}

