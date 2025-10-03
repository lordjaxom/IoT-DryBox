#include <Arduino.h>
#include "main.hpp"

void sensorUpdate(){
    Temperature = dht20.getTemperature();      // Get temperature in degrees Celcius
    Humidity = dht20.getHumidity()*100;        // Get relative humidity
#ifdef Fahrenheit
    Temperature = (Temperature *9/5) + 32;   // Converts temperature to fahrenheit if defined
#endif
}

void heater(){
#ifdef Fahrenheit
    if(Temperature < TargetTemp - 3){
        digitalWrite(Heater, HIGH);            // Turns heating element on if more than 3 degrees Fahrenheit under the target temperature
    }
    if(Temperature > TargetTemp + 3){        // Turns heating element off if more than 3 degrees Fahrenheit over the target temperature
        digitalWrite(Heater, LOW);
    }
#endif
#ifndef Fahrenheit
    if(Temperature < TargetTemp - 1.5){
        analogWrite(Heater, 128);            // Turns heating element on if more than 1.5 degrees celcius under the target temperature
    }
    if(Temperature > TargetTemp + 1.5){      // Turns heating element off if more than 1.5 degrees celcius over the target temperature
        analogWrite(Heater, 0);
    }
#endif
}
