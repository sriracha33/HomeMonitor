#ifndef Monitor_h
#define Monitor_h

#include "Arduino.h"
#include <Adafruit_FONA.h>
#include <OneWire.h>
#include <Adafruit_SSD1306.h>

class Monitor
{ private:
    Adafruit_FONA *_fona;
    OneWire *_onewire;
    Adafruit_SSD1306 *_display;
    
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
    Monitor(Adafruit_FONA *fona, OneWire *onewire, Adafruit_SSD1306 *display);
    float GetVolts();
    uint8_t GetNetworkStatus(byte statword);
    uint8_t GetSignal(byte statword);
};
#endif
