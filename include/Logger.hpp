#ifndef ESP8266_IOT_LOGGER_HPP
#define ESP8266_IOT_LOGGER_HPP

#include <Arduino.h>

#include <stdint.h>
#include <stdio.h>
#include <type_traits>
#include <utility>

namespace detail
{
    class Timestamp
    {
    public:
        Timestamp()
                : timestamp(millis()),
                  ms(timestamp % 1000),
                  s((timestamp /= 1000, timestamp % 60)),
                  m((timestamp /= 60, timestamp % 60)),
                  h((timestamp /= 60, timestamp % 100))
        {
        }

    private:
        unsigned long timestamp;

    public:
        unsigned long ms;
        unsigned long s;
        unsigned long m;
        unsigned long h;
    };
} // namespace detail

#if defined(LOGGER_SERIAL)
#   include "LoggerSerial.hpp"
using LoggerClass = LoggerSerial;
#else
#   include "LoggerNoop.hpp"
using LoggerClass = LoggerNoop;
#endif

extern LoggerClass Logger;

template<typename ...Args>
void log(Args&& ... args)
{
    Logger.log(::detail::Timestamp(), std::forward<Args>(args)...);
}

#endif //ESP8266_IOT_LOGGER_HPP
