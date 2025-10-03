#ifndef IOT_DRYBOX_MAIN_HPP
#define IOT_DRYBOX_MAIN_HPP

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DFRobot_DHT20.h>

#define SDA 4
#define SCL 5

#define Heater 14

extern DFRobot_DHT20 dht20;

extern bool status;

extern int TargetTemp;
extern float Temperature;
extern float Humidity;

extern char TempIntegerDisplay[4];
extern char TempFractionDisplay[4];
extern char HumIntegerDisplay[4];
extern char HumFractionDisplay[4];
extern char TargetTempDisplay[4];

void sensorUpdate();
void heater();

#endif //IOT_DRYBOX_MAIN_HPP