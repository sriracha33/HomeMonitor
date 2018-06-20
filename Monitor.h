#ifndef Monitor_h
#define Monitor_h

#define LOW_VOLTS 0
#define LOW_TEMP 1
#define LOW_BATTERY 2
#define FONA_ON 3
#define GSM_ON 4
#define DISPLAY_ON 5
#define POWERSAVE_MODE 7

#define TEMP_LIMIT 79
#define VOLT_LIMIT 4.3
#define BATTERY_LIMIT 3.5

#include "Arduino.h"
#include <Adafruit_FONA.h>
#include <OneWire.h>
#include <Adafruit_SSD1306.h>

class Monitor
{ private:
    Adafruit_FONA *_fona;
    OneWire *_onewire;
    Adafruit_SSD1306 *_display;
    int _DelayCount = 0;
    
  public:
    /*define display variables*/
    int16_t temperature;    //Temperature in F
    float vbusVolts;        //current voltage of Vbus
    char currentTime[6];    //Time to display
    char dateTime[18];      //Date and Time stamp
    uint8_t battery;        //Battery Percentage
    uint8_t signal;         //Signal Strength
    uint8_t connection;     //Connection status
    char eventTime[18]={0}; //Status Display Message Line 1 (Timestamp)
    char eventText[21]={0}; //Status Display Message Line 2
    byte status=0;          //Status word.  See assignments in define statment

    
    /*methods*/
    Monitor(Adafruit_FONA *fona, OneWire *onewire, Adafruit_SSD1306 *display);
    int GetTemp();
    float GetVolts();
    uint8_t GetBattery();
    uint8_t GetNetworkStatus();
    uint8_t GetSignal();
    void GetTime();
    void SetupTemp();
    void UpdateDisplay();
    void UpdateValues();
    void UpdateStatus();
};
#endif
