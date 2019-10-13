#include "pch.h"
#define BOOST_DECL_EXPORTS
#define BOOST_WARCHIVE_SOURCE
#define BOOST_ARCHIVE_DECL
#include <boost/archive/impl/basic_xml_oarchive.ipp>
#include <boost/archive/impl/xml_oarchive_impl.ipp>

const std::string LogLevel<static_cast<int>(log_level_internal::level_none)>::NAME = "NONE";
const std::string LogLevel<static_cast<int>(log_level_internal::level_error)>::NAME = "ERROR";
const std::string LogLevel<static_cast<int>(log_level_internal::level_warning)>::NAME = "WARNING";
const std::string LogLevel<static_cast<int>(log_level_internal::level_info)>::NAME = "INFO";
const std::string LogLevel<static_cast<int>(log_level_internal::level_debug)>::NAME = "DEBUG";
const std::string LogLevel<static_cast<int>(log_level_internal::level_all)>::NAME = "ALL";


const LogLevel<static_cast<int>(log_level_internal::level_none)>		Log::LEVEL_NONE;
const LogLevel<static_cast<int>(log_level_internal::level_error)>		Log::LEVEL_ERROR;
const LogLevel<static_cast<int>(log_level_internal::level_warning)>	Log::LEVEL_WARNING;
const LogLevel<static_cast<int>(log_level_internal::level_info)>		Log::LEVEL_INFO;
const LogLevel<static_cast<int>(log_level_internal::level_debug)>		Log::LEVEL_DEBUG;
const LogLevel<static_cast<int>(log_level_internal::level_all)>		Log::LEVEL_ALL;

Log::endline Log::endl;

Log::Log()
{
    logging_level = static_cast<int>(log_level_internal::level_none);
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
        on_log(log);
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
    _com_error err(hr);
    crash_error((err.ErrorMessage()), at);
}

std::string LogBlock::get_string() const
{
    return "[" + current_level_name + "] : " + s->str();
}

Log& LogBlock::operator<<(const Log::endline&)
{
    return log << *this;
}

LogBlock& LogBlock::operator<<(char smth)
{
    if (need_logging())
        *s << smth;

    return (*this);
}

LogBlock& LogBlock::operator<<(char* smth)
{
    if (need_logging())
        *s << smth;

    return (*this);
}

LogBlock& LogBlock::operator<<(const char* smth)
{
    if (need_logging())
        *s << smth;

    return *this;
}

LogBlock& LogBlock::operator<<(wchar_t smth)
{
    if (need_logging())
        *s << smth;

    return (*this);
}

LogBlock& LogBlock::operator<<(wchar_t* smth)
{
    if (need_logging())
        *s << convert(smth);

    return (*this);
}

LogBlock& LogBlock::operator<<(const wchar_t* smth)
{
    if (need_logging())
        *s << convert(smth);

    return *this;
}

LogBlock::LogBlock(Log& output, int level) : log(output)
{
    log_level = level;
    current_level = static_cast<int>(log_level_internal::level_info);
    current_level_name = LogLevel<static_cast<int>(log_level_internal::level_info)>::NAME;
    s.reset(new std::ostringstream());
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
    //  auto now_c = std::chrono::system_clock::to_time_t(now);
    *s << delta << ' ';
}

LogListener::~LogListener()
{
    //on_log_removed();
}

LogListener::LogListener()
{
//   Log::get().listeners.insert(this);
    active = true;
    Log::get().on_log.register_handler(this, std::bind(&LogListener::on_log, this, std::placeholders::_1));
}


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
    log_file.open(file_name,  ios::trunc);
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