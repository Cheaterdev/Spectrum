export module Core:Exceptions;

//#include <boost/stacktrace.hpp>
import stl.core;
//import <stacktrace>;
export namespace Exceptions
{
	using stack_trace = int;//std::stacktrace;// boost::stacktrace::stacktrace;

	class Exception : public std::exception
	{
		stack_trace stack;
	public:
		Exception(std::string_view wtf);
		stack_trace get_stack_trace();
	};

	stack_trace get_stack_trace();	
}


