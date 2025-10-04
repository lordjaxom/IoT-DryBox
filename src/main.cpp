#include "IoT.hpp"
#include "Controller.hpp"
#include "Debounce.hpp"
#include "DHT20.hpp"
#include "Display.hpp"
#include "DS18B20.hpp"
#include "Gpio.hpp"
#include "Mqtt.hpp"
#include "Output.hpp"
#include "PushButton.hpp"

#include "Heating.h"

static constexpr auto client_id = "Workshop-Appliance-DryBox";
static constexpr auto wifi_ssid = "VillaKunterbunt";
static constexpr auto wifi_password = "sacomoco02047781";
static constexpr auto mqtt_server = "openhab";

IoTClass IoT{client_id, wifi_ssid, wifi_password};
Mqtt mqtt{mqtt_server};

static constexpr uint8_t onOffButtonPin = 0;
static constexpr uint8_t upButtonPin = 2;
static constexpr uint8_t downButtonPin = 16;
static constexpr uint8_t ds18b20Pin = 13;
static constexpr uint8_t heaterPin = 14;

PushButton onOffButton{debounce(gpioInput(onOffButtonPin))};
PushButton upButton{debounce(gpioInput(upButtonPin))};
PushButton downButton{debounce(gpioInput(downButtonPin))};
AnalogOutput heater{analogOutput(heaterPin)};

DHT20 dht20;
DS18B20 ds18b20{ds18b20Pin};
Display display;

Controller controller{
    mqtt,
    onOffButton,
    upButton,
    downButton,
    heater,
    dht20,
    ds18b20,
    display
};

void setup()
{
    Wire.setClock(400'000);
    IoT.begin();
}

void loop()
{
    IoT.loop();
}

extern "C" {
float readTemperature()
{
    return ds18b20.getTemperature();
}

void setHeater(int const level)
{
    heater.set(level);
}

void reportValues(char const* text)
{
    mqtt.publishTelemetry("HEATING", text);
}
} // extern "C"
