#include "pch.h"

#include <boost/stacktrace.hpp>

export module Exceptions;

export namespace Exceptions
{
	export using stack_trace = boost::stacktrace::stacktrace;


	export class Exception : public std::exception
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

	Exception::Exception(std::string_view wtf) : std::exception((std::string(wtf) + "\n At:" + boost::stacktrace::to_string(stack)).c_str())
	{
	}

	stack_trace Exception::get_stack_trace()
	{
		return 	stack;
	}


	stack_trace get_stack_trace()
	{
		return boost::stacktrace::stacktrace();
	}
}
