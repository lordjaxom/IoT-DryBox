#ifndef ESP8266_IOT_LOGGERNOOP_HPP
#define ESP8266_IOT_LOGGERNOOP_HPP

class LoggerNoop
{
public:
    void begin() {}

    template<typename... Args>
    void log(Args&& ... args) {}

    void flush() {}
};

#endif //ESP8266_IOT_LOGGERNOOP_HPP
