#pragma once
#pragma warning( push )
#pragma warning( disable : 4512 )

#include "simple_archive.h"
//class LogListener;
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

static auto start_time = std::chrono::high_resolution_clock::now();

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
        LogBlock & operator<<(T smth)
        {
            if (need_logging())
                *s << smth;

            return *this;
        }

        template < class T,
                   typename std::enable_if < !(std::is_scalar <T>::value || !need_log_serialize<T>::value) >::type* = nullptr >
        LogBlock & operator<<(const T& smth)
        {
            if (need_logging())
            {
                simple_log_archive oa(*s);
                //	*s << "{ ";
                oa << smth;
                //	*s << "} ";
            }

            return *this;
        }



        LogBlock& operator<<(const std::wstring& smth)
        {
            if (need_logging())
            {
                simple_log_archive oa(*s);
                //	*s << "{ ";
                oa << convert(smth);
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


class LogListener: public Events::prop_handler
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
class ClassLogger: public Singleton<ClassLogger<T>>
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

#pragma warning( pop )



#define EVENT(x) {Log::get()<<"EVENT: "<<#x<< " time: "<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start_time).count()<<Log::endl;}
/*
TODO?
class NetworkLogger;
class HTMLLogger;
class VisualLogger;
class ConsoleLogger;
*/