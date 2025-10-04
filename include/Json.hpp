#ifndef ESP8266_IOT_JSON_HPP
#define ESP8266_IOT_JSON_HPP

#include <ArduinoJson.h>

ARDUINOJSON_BEGIN_PUBLIC_NAMESPACE
        inline void strAppend(String& result, JsonDocument const& arg)
        {
            serializeJson(arg, result);
        }
ARDUINOJSON_END_PUBLIC_NAMESPACE

#endif // ESP8266_IOT_JSON_HPP
