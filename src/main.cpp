#include "IoT.hpp"
#include "Controller.hpp"
#include "Debounce.hpp"
#include "DHT20.hpp"
#include "Display.hpp"
#include "DS18B20.hpp"
#include "Gpio.hpp"
#include "Mqtt.hpp"
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

DHT20 dht20;
DS18B20 ds18b20{ds18b20Pin};
Display display;

Controller controller{
    mqtt,
    onOffButton,
    upButton,
    downButton,
    dht20,
    ds18b20,
    display
};

void setup()
{
    pinMode(heaterPin, OUTPUT);
    Wire.setClock(400'000);
    IoT.begin();
}

void loop()
{
    IoT.loop();

    //
    // // Heating system ON:
    // while (status == true) {
    //     unsigned long currentMillis = millis();
    //
    //     if (digitalRead(Button1) == LOW) {
    //         // Action button 1: switch heating system from on to off
    //         status = !status;
    //
    //         sensorUpdate();
    //         digitalWrite(Heater, LOW);
    //         display.clearDisplay();
    //         drawStatus();
    //         drawTemperature();
    //         drawHumidity();
    //         display.display();
    //         delay(200);
    //     }
    //     if (digitalRead(Button2) == LOW) {
    //         // Action button 2: increase target temperature
    //         TargetTemp = TargetTemp + 1;
    //         previousMillis = currentMillis;
    //         display.clearDisplay();
    //         drawTargetTemperature();
    //         display.display();
    //         delay(200);
    //     }
    //     if (digitalRead(Button3) == LOW) {
    //         // Action button 3: decrease target temperature
    //         TargetTemp = TargetTemp - 1;
    //         previousMillis = currentMillis;
    //         display.clearDisplay();
    //         drawTargetTemperature();
    //         display.display();
    //         delay(200);
    //     }
    //     if (currentMillis - previousMillis >= 100) {
    //         previousMillis = currentMillis;
    //         sensorUpdate();
    //
    //         display.clearDisplay();
    //         drawStatus();
    //         drawTemperature();
    //         drawHumidity();
    //         display.display();
    //         heater();
    //     }
    // }
    //
    // // Heating system OFF:
    // while (status == false) {
    //     unsigned long currentMillis = millis();
    //
    //     if (digitalRead(Button1) == LOW) {
    //         // Action button 1: switch heating system from off to on
    //         status = !status;
    //         sensorUpdate();
    //         display.clearDisplay();
    //         drawStatus();
    //         drawTemperature();
    //         drawHumidity();
    //         display.display();
    //         delay(200);
    //     }
    //     if (currentMillis - previousMillis >= 100) {
    //         previousMillis = currentMillis;
    //         sensorUpdate();
    //
    //         display.clearDisplay();
    //         drawStatus();
    //         drawTemperature();
    //         drawHumidity();
    //         display.display();
    //     }
    // }
}

extern "C" {

    float readTemperature()
    {
        return ds18b20.getTemperature();
    }

    void setHeater(int const level)
    {
        analogWrite(heaterPin, level);
    }

    void reportValues(char const *text)
    {
        mqtt.publishTelemetry("HEATING", text);
    }

} // extern "C"