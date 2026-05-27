module Core:Log;

import stl.core;
import stl.threading;
import :Utils;
import windows;

static auto start_time = std::chrono::high_resolution_clock::now();

void EVENT(std::string_view s)
{
	Log::get() << "EVENT: " << s << " time: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count() << Log::endl;
}


const std::string LogLevel<log_level_internal::level_none>::NAME    = "NONE";
const std::string LogLevel<log_level_internal::level_error>::NAME   = "ERROR";
const std::string LogLevel<log_level_internal::level_warning>::NAME = "WARNING";
const std::string LogLevel<log_level_internal::level_info>::NAME    = "INFO";
const std::string LogLevel<log_level_internal::level_debug>::NAME   = "DEBUG";
const std::string LogLevel<log_level_internal::level_all>::NAME     = "ALL";


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
}

Log& Log::operator<<(const LogBlock& log)
{
	m.lock();

	if (log.need_logging())
	{
		on_log(&log);
	}

	m.unlock();
	return *this;
}

void Log::crash_error(std::string message, std::string_view at)
{
	(*this) << LEVEL_ERROR << message << " at: " << at << endl;
}

void Log::crash_error(HRESULT hr, std::string_view at /*= ""*/)
{
	std::string message = std::system_category().message(hr);
	(*this) << LEVEL_ERROR << message << " at: " << at << endl;
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


shared_data::shared_data() : archive(s)
{
}

LogBlock::LogBlock(Log& output, log_level_internal level) : log(output)
{
	log_level = level;
	current_level = log_level_internal::level_info;
	current_level_name = LogLevel<log_level_internal::level_info>::NAME;

	data = std::make_shared<shared_data>();
	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
	data->s << delta << ' ';
}

LogListener::LogListener()
{
	active = true;

	typename Events::Event<const LogBlock*>::func_type f = [this](const LogBlock* v) { on_log(*v); };
	Log::get().on_log.register_handler(this, f);
}

LogListener::~LogListener()
{
}

FileTXTLogger::~FileTXTLogger()
{
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
		SetConsoleTitleW(L"Spectrum Console");
	}
}

ConsoleLogger::~ConsoleLogger()
{
	if (GetStdHandle(STD_OUTPUT_HANDLE))
		FreeConsole();
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
			color |= FOREGROUND_INTENSITY;

		SetConsoleTextAttribute(hConsole, color);
		// Print message
		auto str = log.get_string() + "\n";
		WriteConsoleA(hConsole, str.c_str(), static_cast<DWORD>(str.length()), nullptr, nullptr);
	}
}
