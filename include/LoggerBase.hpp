#ifndef ESP8266_IOT_LOGGERBASE_HPP
#define ESP8266_IOT_LOGGERBASE_HPP

#include <utility>

template<typename Impl>
class LoggerBase
{
public:
    void log()
    {
        static_cast<Impl*>(this)->flush();
    }

    template<typename Arg0, typename ...Args>
    void log(Arg0&& arg0, Args&& ... args)
    {
        static_cast<Impl*>(this)->append(std::forward<Arg0>(arg0));
        log(std::forward<Args>(args)...);
    }
};

#endif //ESP8266_IOT_LOGGERBASE_HPP
