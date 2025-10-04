#include "IoT.hpp"
#include "DS18B20.hpp"

static DS18B20* instance;

void DS18B20::handleTemperatureChange(int, int32_t const temperature)
{
    instance->temperature_ = instance->nonBlocking_.rawToCelsius(temperature);
}

DS18B20::DS18B20(uint8_t const pin) noexcept
    : oneWire_{pin},
      sensor_{&oneWire_},
      nonBlocking_{&sensor_},
      looped_{IoT.loopEvent.subscribe([this] { nonBlocking_.update(); })}
{
    instance = this;
    IoT.beginEvent += [this] { nonBlocking_.begin(NonBlockingDallas::resolution_12, 1000); };
    nonBlocking_.onTemperatureChange(&DS18B20::handleTemperatureChange);
}
