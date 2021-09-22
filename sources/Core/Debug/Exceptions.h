
namespace Exceptions
{
	using stack_trace = boost::stacktrace::stacktrace;

	class Exception : public std::exception
	{
		stack_trace stack;
	public:
		Exception(std::string_view wtf);
		stack_trace get_stack_trace();
	};

	stack_trace get_stack_trace();
}


#ifdef SPECTRUM_ENABLE_EXEPTIONS
#define SPECTRUM_TRY try{
#define SPECTRUM_CATCH }catch (const std::system_error& e){Log::get() << Log::LEVEL_ERROR << e.what() << Log::endl;}
#else
#define SPECTRUM_TRY {
#define SPECTRUM_CATCH }
#endif