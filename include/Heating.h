#ifndef IOT_DRYBOX_HEATING_H
#define IOT_DRYBOX_HEATING_H

#ifdef __cplusplus
extern "C" {
#endif

void controlHeater(float Ts);

float readTemperature(void);

void setHeater(int level);

void reportValues(char const *text);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
