#ifndef ESP8266_IOT_DS18B20_HPP
#define ESP8266_IOT_DS18B20_HPP

#include <DallasTemperature.h>
#include <NonBlockingDallas.h>

#include "Event.hpp"

class DS18B20
{
    static void handleTemperatureChange(int device, int32_t temperature);

public:
    explicit DS18B20(uint8_t pin) noexcept;
    DS18B20(DS18B20 const&) = delete;

    [[nodiscard]] float getTemperature() const { return temperature_;}

private:
    OneWire oneWire_;
    DallasTemperature sensor_;
    NonBlockingDallas nonBlocking_;
    Subscription looped_;
    float temperature_{};
};

#endif

