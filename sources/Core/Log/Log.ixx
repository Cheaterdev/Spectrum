module;


#define BOOST_DECL_S
#define BOOST_WARCHIVE_SOURCE
#define BOOST_ARCHIVE_DECL

export module Log;

import Data;
import stl.threading;
import stl.core;
import stl.memory;
import Singleton;
import simple_log_archive;
import Events;
import Utils;
import windows;

export {

	class LogBlock;

	enum class log_level_internal : int { level_none = 0, level_error = 1, level_warning = 2, level_info = 3, level_debug = 4, level_all = 65536, GENERATE_OPS };

	template <log_level_internal L>
	struct LogLevel
	{
		static const std::string NAME;
	};


	class Log : public Singleton<Log>
	{
		friend class Singleton<Log>;
		friend class LogBlock;

		struct endline {};
		std::mutex m;
		log_level_internal logging_level;
	protected:
		Log();
		virtual ~Log();
	public:

		template<log_level_internal L>
		void set_logging_level(const LogLevel<L>&)
		{
			logging_level = L;
		}

		Events::Event<const LogBlock*> on_log;


		static endline endl;
		static const LogLevel<log_level_internal::level_none>		LEVEL_NONE;
		static const LogLevel<log_level_internal::level_error>	    LEVEL_ERROR;
		static const LogLevel<log_level_internal::level_warning>	LEVEL_WARNING;
		static const LogLevel<log_level_internal::level_info>		LEVEL_INFO;
		static const LogLevel<log_level_internal::level_debug>	    LEVEL_DEBUG;
		static const LogLevel<log_level_internal::level_all>		LEVEL_ALL;

		template<class T>
		LogBlock operator<<(const  T& smth);
		Log& operator<<(const LogBlock& smth);

		// this functions are last what user sees
		virtual void crash_error(std::string message, std::string at = "");
		virtual void crash_error(HRESULT hr, std::string at = "");
	};
	struct shared_data
	{
		std::stringstream s;
		simple_log_archive archive;

		shared_data();
	};

	class LogBlock
	{


		friend class Log;

		Log& log;

		std::shared_ptr<shared_data> data;
		log_level_internal current_level;
		log_level_internal log_level;
		std::string current_level_name;
		bool need_logging() const
		{
			return (current_level <= log_level);
		}
	public:

		LogBlock(Log& output, log_level_internal level);
		std::string get_level() const
		{
			return current_level_name;
		}
		log_level_internal get_level_internal() const
		{
			return current_level;
		}
		template<log_level_internal L>
		LogBlock& operator<<(const LogLevel<L>&)
		{
			current_level = L;
			current_level_name = std::string(LogLevel<L>::NAME);
			return *this;
		}

		LogBlock& operator<<(std::string_view);
		LogBlock& operator<<(std::wstring_view);

		template < NonString T>
		LogBlock& operator<<(const T& smth)
		{
			if (need_logging())
			{
				data->archive << smth;
			}

			return *this;
		}


		Log& operator<<(const Log::endline& smth);

		std::string get_string() const;
	};



	class LogListener : public Events::prop_handler
	{
		virtual void on_log(const LogBlock&) = 0;

		//  friend class Log;
		bool active;
	protected:
		//   virtual void on_log_removed();
		LogListener() {
			//   Log::get().listeners.insert(this);
			active = true;

				typename Events::Event<const LogBlock*>::func_type f = [this](const LogBlock* v) { on_log(*v); };
				Log::get().on_log.register_handler(this, f);

		}

		virtual ~LogListener();
	};

	class FileTXTLogger : public Singleton<FileTXTLogger>, public LogListener
	{
		friend class Singleton<FileTXTLogger>;

		std::ofstream log_file;

		FileTXTLogger(std::string file_name = "log.txt");
		virtual ~FileTXTLogger();

		void on_log(const LogBlock& log);
	};


	class VSOutputLogger : public Singleton<VSOutputLogger>, public LogListener
	{
		friend class Singleton<VSOutputLogger>;

		std::ofstream log_file;
		//virtual ~VSOutputLogger() { on_log_removed(); };
		void on_log(const LogBlock& log);
	};

	class ConsoleLogger : public Singleton<ConsoleLogger>, public LogListener
	{
		friend class Singleton<ConsoleLogger>;
		bool even;
		log_level_internal prev_log_level;
		ConsoleLogger();
		virtual ~ConsoleLogger();
		void on_log(const LogBlock& log);
	};

	// use this logger for systems so we can change logging level for them
	template <class T>
	class ClassLogger : public Singleton<ClassLogger<T>>
	{
		log_level_internal logging_level;
		friend class Singleton<ClassLogger<T>>;
	protected:
		ClassLogger()
		{
			logging_level = log_level_internal::level_none;
		}
		virtual ~ClassLogger()
		{
		}
	public:

		template<log_level_internal L>
		void set_logging_level(const LogLevel<L>&)
		{
			logging_level = L;
		}

		template<class T>
		LogBlock operator<<(const  T& smth)
		{

			auto block = LogBlock(Log::get(), logging_level);
			block << smth;

			return block;
		}
	};
	void EVENT(std::string_view s);

	template<class T>
	LogBlock Log::operator<<(const T& smth)
	{
		return LogBlock(*this, logging_level) << smth;
	}

}
module: private;


static auto start_time = std::chrono::high_resolution_clock::now();

void EVENT(std::string_view s)
{
	Log::get() << "EVENT: " << s << " time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << Log::endl;
}


/*
TODO?
class NetworkLogger;
class HTMLLogger;
class VisualLogger;
class ConsoleLogger;
*/




const std::string LogLevel<log_level_internal::level_none>::NAME = "NONE";
const std::string LogLevel<log_level_internal::level_error>::NAME = "ERROR";
const std::string LogLevel<log_level_internal::level_warning>::NAME = "WARNING";
const std::string LogLevel<log_level_internal::level_info>::NAME = "INFO";
const std::string LogLevel<log_level_internal::level_debug>::NAME = "DEBUG";
const std::string LogLevel<log_level_internal::level_all>::NAME = "ALL";


const LogLevel<log_level_internal::level_none>		Log::LEVEL_NONE;
const LogLevel<log_level_internal::level_error>		Log::LEVEL_ERROR;
const LogLevel<log_level_internal::level_warning>	Log::LEVEL_WARNING;
const LogLevel<log_level_internal::level_info>		Log::LEVEL_INFO;
const LogLevel<log_level_internal::level_debug>		Log::LEVEL_DEBUG;
const LogLevel<log_level_internal::level_all>		Log::LEVEL_ALL;

Log::endline Log::endl;

Log::Log()
{
	logging_level = log_level_internal::level_none;
}

Log::~Log()
{
	//   m.lock();
		// auto listeners_copy = listeners;
	//   for (auto l : listeners_copy)
		//     l->on_log_removed();
		//  m.unlock();
}

Log& Log::operator<<(const LogBlock& log)
{
	m.lock();

	if (log.need_logging())
	{
		/*for (auto l : listeners)
			l->on_log(log);

		for (auto l : listener_functions)
			l(log);*/
		on_log(&log);
	}

	m.unlock();
	return *this;
}

void Log::crash_error(std::string message, std::string at)
{
	(*this) << LEVEL_ERROR << message << " at: " << at << endl;
}

void Log::crash_error(HRESULT hr, std::string at /*= ""*/)
{
	//_com_error err(hr);
	//crash_error((err.ErrorMessage()), at);
}

std::string LogBlock::get_string() const
{
	return "[" + current_level_name + "] : " + data->s.str();
}

Log& LogBlock::operator<<(const Log::endline&)
{
	return log << *this;
}

LogBlock& LogBlock::operator<<(std::string_view smth)
{
	if (need_logging())
		data->s << smth;

	return (*this);
}

LogBlock& LogBlock::operator<<(std::wstring_view smth)
{
	if (need_logging())
		data->s << convert(smth);

	return (*this);
}


shared_data::shared_data() :archive(s)
{

}

LogBlock::LogBlock(Log& output, log_level_internal level) : log(output)
{
	log_level = level;
	current_level = log_level_internal::level_info;
	current_level_name = LogLevel<log_level_internal::level_info>::NAME;

	data = std::make_shared<shared_data>();
	//  s.reset(new std::ostringstream());
	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
	//  auto now_c = std::chrono::system_clock::to_time_t(now);
	data->s << delta << ' ';
}

LogListener::~LogListener()
{
	//on_log_removed();
}/*

LogListener::LogListener()
{
	//   Log::get().listeners.insert(this);
	active = true;

	std::function<void(LogBlock* v)> f = [this](LogBlock* v) { on_log(*v); };
	Log::get().on_log.register_handler(this, f);

}
*/

FileTXTLogger::~FileTXTLogger()
{
	// on_log_removed();
	if (log_file.is_open())
		log_file.close();
}

void FileTXTLogger::on_log(const LogBlock& log)
{
	log_file << log.get_string() << std::endl;
	log_file.flush();
}

FileTXTLogger::FileTXTLogger(std::string file_name /*= "log.txt"*/)
{
	//	DeleteFile(file_name.c_str());
	log_file.open(file_name, std::ios::trunc);
}

void VSOutputLogger::on_log(const LogBlock& log)
{
	OutputDebugStringA((log.get_string() + "\n").c_str());
}

ConsoleLogger::ConsoleLogger()
{
	even = true;

	if (!GetStdHandle(STD_OUTPUT_HANDLE))
	{
		AllocConsole();
		//SetConsoleCtrlHandler(consoleSignalDispatcher, TRUE);
		SetConsoleTitleW(L"Spectrum Console");
	}
}

ConsoleLogger::~ConsoleLogger()
{
	if (GetStdHandle(STD_OUTPUT_HANDLE))
		FreeConsole();

	//	on_log_removed();
}

void ConsoleLogger::on_log(const LogBlock& log)
{
	even = !even;

	if (prev_log_level != log.get_level_internal())
	{
		even = true;
		prev_log_level = log.get_level_internal();
	}

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hConsole != INVALID_HANDLE_VALUE)
	{
#define FOREGROUND_WHITE	FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_BLUE
#define FOREGROUND_YELLOW	FOREGROUND_GREEN|FOREGROUND_RED
#define FOREGROUND_MAGENTA	FOREGROUND_BLUE|FOREGROUND_RED
#define FOREGROUND_CYAN		FOREGROUND_BLUE|FOREGROUND_GREEN
		// Setup color
		WORD color;

		switch (log.get_level_internal())
		{
		case log_level_internal::level_error:
			color = FOREGROUND_RED | FOREGROUND_INTENSITY;
			break;

		case log_level_internal::level_warning:
			color = FOREGROUND_YELLOW;
			break;

		case log_level_internal::level_info:
			color = FOREGROUND_WHITE;
			break;

		case log_level_internal::level_debug:
			color = FOREGROUND_GREEN;
			break;

		case log_level_internal::level_none:
		default:
			color = FOREGROUND_WHITE;
			break;
		}

		if (even)
			color |= FOREGROUND_INTENSITY;// | BACKGROUND_INTENSITY;

		SetConsoleTextAttribute(hConsole, color);
		// Print message
		auto str = log.get_string() + "\n";
		WriteConsoleA(hConsole, str.c_str(), static_cast<DWORD>(str.length()), NULL, NULL);
	}
}