export module Core:Exceptions;

import stl.core;
export namespace Exceptions
{
	using stack_trace = std::stacktrace;

	class Exception : public std::exception
	{
		stack_trace stack;
	public:
		Exception(std::string_view wtf);
		stack_trace get_stack_trace();
	};

	stack_trace get_stack_trace();
}
