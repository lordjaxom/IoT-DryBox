#ifndef ESP8266_IOT_GPIO_HPP
#define ESP8266_IOT_GPIO_HPP

#include <functional>

std::function<bool()> gpioInput(uint8_t pin, bool pullUp = true);
std::function<void(bool value)> gpioOutput(uint8_t pin, bool invert = false);

#endif // ESP8266_IOT_GPIO_HPP
