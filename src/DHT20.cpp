#include "IoT.hpp"
#include "DHT20.hpp"

DHT20::DHT20() noexcept
{
    IoT.beginEvent += [this] { sensor_.begin(); };
}

void DHT20::update()
{
    temperature_ = sensor_.getTemperature();
    humidity_ = sensor_.getHumidity();
}
