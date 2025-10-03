#ifndef ESP8266_IOT_DHT20_HPP
#define ESP8266_IOT_DHT20_HPP

#include <DFRobot_DHT20.h>

class DHT20
{
public:
    DHT20() noexcept;
    DHT20(DHT20 const&) = delete;

    [[nodiscard]] float getTemperature() const { return temperature_; }
    [[nodiscard]] float getHumidity() const { return humidity_; }

    void update();

private:
    DFRobot_DHT20 sensor_;
    float temperature_{};
    float humidity_{};
};

#endif