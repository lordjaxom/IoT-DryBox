#ifndef ESP8266_IOT_DISPLAY_HPP
#define ESP8266_IOT_DISPLAY_HPP

#include <Adafruit_SSD1306.h>

class Display
{
    static constexpr uint8_t width = 128;
    static constexpr uint8_t height = 64;
    static constexpr uint8_t oledReset = 4;
    static constexpr uint8_t address = 0x3C;

public:
    Display();
    Display(Display const&) = delete;

    void showStandbyPanel(float temperature, float humidity);
    void showHeatingPanel(float temperature, float humidity);
    void showSetpointPanel(float setpoint);

private:
    void begin();

    Adafruit_SSD1306 display_{width, height, &Wire, -1};
};

#endif