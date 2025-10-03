#ifndef ESP8266_IOT_DS18B20_HPP
#define ESP8266_IOT_DS18B20_HPP

#include "DallasTemperature.h"

class DS18B20
{
public:
    explicit DS18B20(uint8_t pin) noexcept;
    DS18B20(DS18B20 const&) = delete;

    [[nodiscard]] float getTemperature() const { return temperature_;}

    void update();

private:
    OneWire oneWire_;
    DallasTemperature sensor_;
    float temperature_{};
};

#endif