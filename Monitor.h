#ifndef Monitor_h
#define Monitor_h

#include "Arduino.h"
class Monitor
{
    int var1;
  public:
    /*define display variables*/
    int16_t temperature;  //Temperature in F
    float vbusVolts;      //current voltage of Vbus
    char currentTime[6];  //Time to display
    char dateTime[18];    //Date and Time stamp
    uint8_t battery;      //Battery Percentage
    uint8_t signal;       //Signal Strength
    uint8_t connection;   //Connection status
    
    /*methods*/
    Monitor();
    float GetVolts();
};
#endif
