/*
Code for the (Un)original Prusa Drybox Heater
Written by Bram Elema, Creative Commons BY-NC-SA

REQUIRES the following Arduino libraries:
 - Adafruit_GFX Library: https://github.com/adafruit/Adafruit-GFX-Library
 - Adafruit_SSD1306 Library: https://github.com/adafruit/Adafruit_SSD1306
 - DFRobot_DHT20 Sensor Library: https://github.com/DFRobot/DFRobot_DHT20
*/

#include <DFRobot_DHT20.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "main.hpp"

#include "IoT.hpp"
#include "Controller.hpp"
#include "Debounce.hpp"
#include "Display.hpp"
#include "Gpio.hpp"
#include "Mqtt.hpp"
#include "PushButton.hpp"

static constexpr auto client_id = "Workshop-Appliance-DryBox";
static constexpr auto wifi_ssid = "VillaKunterbunt";
static constexpr auto wifi_password = "sacomoco02047781";
static constexpr auto mqtt_server = "openhab";

IoTClass IoT{client_id, wifi_ssid, wifi_password};
Mqtt mqtt{mqtt_server};

static constexpr uint8_t onOffButtonPin = 0;
static constexpr uint8_t upButtonPin = 2;
static constexpr uint8_t downButtonPin = 16;
static constexpr uint8_t heaterPin = 14;

PushButton onOffButton{debounce(gpioInput(onOffButtonPin))};
PushButton upButton{debounce(gpioInput(upButtonPin))};
PushButton downButton{debounce(gpioInput(downButtonPin))};

Display display;
Controller controller{mqtt, onOffButton, display};

// Uncomment the next line if you want the temperature to be displayed in Fahrenheit, it is displayed in Celcius by default
// #define Fahrenheit

DFRobot_DHT20 dht20;
OneWire oneWire{13};
DallasTemperature ds18b20{&oneWire};

unsigned long previousMillis = 0;
bool status = false;

int TargetTemp = 45;
float Temperature;
float Humidity;

char TempIntegerDisplay[4];
char TempFractionDisplay[4];
char HumIntegerDisplay[4];
char HumFractionDisplay[4];
char TargetTempDisplay[4];

void setup()
{
    IoT.begin();

    //Wire.begin(4, 5);
    dht20.begin();
    ds18b20.begin();
//
//     Wire.begin(SDA, SCL);
//
//     // Initialize buttons and heater
//     pinMode(Button1, INPUT_PULLUP);
//     pinMode(Button2, INPUT_PULLUP);
//     pinMode(Button3, INPUT_PULLUP);
//     pinMode(Heater, OUTPUT);
//
//     // Check if sensor and display are working
//     if (dht20.begin()) {
//         Serial.println("Initialize sensor failed");
//         delay(1000);
//     }
//     if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
//         Serial.println("Initialize display failed");
//         delay(1000);
//     }
//
// #ifdef Fahrenheit
//     TargetTemp = 113;
// #endif
//
//     drawLogo();
//     delay(1200);
//     sensorUpdate();
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
