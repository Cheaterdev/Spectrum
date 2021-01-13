#include "pch.h"

//#include "simple_archive.h"

export module Logs;


import Singletons;
import EventsProps;


static auto start_time = std::chrono::high_resolution_clock::now();

export
{
    class LogBlock;

    enum class log_level_internal : int { level_none = 0, level_error = 1, level_warning = 2, level_info = 3, level_debug = 4, level_all = 65536, GENERATE_OPS };

    template <log_level_internal L>
    struct LogLevel
    {
        static const std::string NAME;
    };
    template <class T>
    struct need_log_serialize
    {
        static const bool value = true;
    };

   
    class Log : public Singleton<Log>
    {
        friend class Singleton<Log>;
        //friend class LogListener;
        friend class LogBlock;


        struct endline {};
        std::mutex m;
        //   std::set<LogListener*> listeners;
        //  std::vector< std::function<void(const LogBlock&)>> listener_functions;

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

        Events::Event<const LogBlock&> on_log;


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

        /*  void register_listener(std::function<void(const LogBlock&)> f)
          {
              listener_functions.push_back(f);
          }*/
    };

    class LogBlock
    {
        friend class Log;

        Log& log;
        std::shared_ptr<std::ostringstream> s;
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

        LogBlock& operator<<(const char* smth);

        LogBlock& operator<<(char* smth);

        LogBlock& operator<<(char smth);

        LogBlock& operator<<(const wchar_t* smth);

        LogBlock& operator<<(wchar_t* smth);

        LogBlock& operator<<(wchar_t smth);

        template < class T,
            typename std::enable_if < std::is_scalar <T>::value || !need_log_serialize<T>::value >::type* = nullptr >
            LogBlock& operator<<(T smth)
        {
            if (need_logging())
                *s << smth;

            return *this;
        }

        template < class T,
            typename std::enable_if < !(std::is_scalar <T>::value || !need_log_serialize<T>::value) >::type* = nullptr >
            LogBlock& operator<<(const T& smth)
        {
            if (need_logging())
            {
                //     simple_log_archive oa(*s);
                     //	*s << "{ ";
               //      oa << smth;
                     //	*s << "} ";
            }

            return *this;
        }



        LogBlock& operator<<(const std::wstring& smth)
        {
            if (need_logging())
            {
                //     simple_log_archive oa(*s);
                     //	*s << "{ ";
               //      oa << convert(smth);
                     //	*s << "} ";
            }

            return *this;
        }


        Log& operator<<(const Log::endline& smth);

        std::string get_string() const;
    };

    template<class T>
    LogBlock Log::operator<<(const T& smth)
    {
        return LogBlock(*this, logging_level) << smth;
    }


    class LogListener : public Events::prop_handler
    {
        virtual void on_log(const LogBlock&) = 0;

        //  friend class Log;
        bool active;
    protected:
        //   virtual void on_log_removed();
        LogListener();
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
            return LogBlock(Log::get(), logging_level) << smth;
        }
    };



}


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
/*
LogBlock& LogBlock::operator<<(wchar_t smth)
{
    if (need_logging())
        *s << smth;

    return (*this);
}*/

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

LogBlock::LogBlock(Log& output, log_level_internal level) : log(output)
{
    log_level = level;
    current_level = log_level_internal::level_info;
    current_level_name = LogLevel<log_level_internal::level_info>::NAME;
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
    log_file.open(file_name, ios::trunc);
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