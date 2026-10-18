export module Core:Exceptions;

import stl.core;
export namespace Exceptions
{
#if __has_include(<stacktrace>)
	using stack_trace = std::stacktrace;
#else
	// libc++ (Android NDK) has no <stacktrace>.
	struct stack_trace {};
	template<class Archive> void serialize(Archive&, stack_trace&) {}   // cereal needs one for Object's member
#endif

	class Exception : public std::exception
	{
		stack_trace stack;
		std::string message;   // not std::exception(const char*): that MSVC extension doesn't exist in libc++
	public:
		const char* what() const noexcept override { return message.c_str(); }
		Exception(std::string_view wtf);
		stack_trace get_stack_trace();
	};

	stack_trace get_stack_trace();
}
