#include <Arduino.h>

#include "Gpio.hpp"
#include "IoT.hpp"

using namespace std;

std::function<bool()> gpioInput(uint8_t const pin, bool const pullUp)
{
    IoT.beginEvent += [=] { pinMode(pin, static_cast<uint8_t>(pullUp ? INPUT_PULLUP : INPUT)); };
    return [=] { return digitalRead(pin) == (pullUp ? LOW : HIGH); };
}

std::function<void(bool value)> gpioOutput(uint8_t const pin, bool const invert)
{
    IoT.beginEvent += [=] { pinMode(pin, OUTPUT); };
    return [=](bool const value) { digitalWrite(pin, static_cast<uint8_t>(value == invert ? LOW : HIGH)); };
}

std::function<void(uint8_t value)> analogOutput(uint8_t const pin)
{
    IoT.beginEvent += [=] { pinMode(pin, OUTPUT); };
    return [=](uint8_t const value) { analogWrite(pin, value); };
}