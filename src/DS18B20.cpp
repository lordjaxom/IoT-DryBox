#include "IoT.hpp"
#include "DS18B20.hpp"

DS18B20::DS18B20(uint8_t const pin) noexcept
        : oneWire_{pin},
          sensor_{&oneWire_}
{
    IoT.beginEvent += [this] { sensor_.begin(); };
}

void DS18B20::update()
{
    sensor_.requestTemperatures();
    temperature_ = sensor_.getTempCByIndex(0);
}
