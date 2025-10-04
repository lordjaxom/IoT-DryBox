#ifndef ESP8266_IOT_DHT20_HPP
#define ESP8266_IOT_DHT20_HPP

#include <DFRobot_DHT20.h>

#include "Timer.hpp"

class DHT20
{
    static constexpr uint64_t updateDelay = 1000;

public:
    DHT20() noexcept;
    DHT20(DHT20 const&) = delete;

    [[nodiscard]] float getTemperature() const { return temperature_; }
    [[nodiscard]] float getHumidity() const { return humidity_; }

private:
    void update();

    DFRobot_DHT20 sensor_;
    Timer updateTimer_;
    float temperature_{};
    float humidity_{};
};

#endif