#include "Arduino.h"
#include "Monitor.h"

Monitor::Monitor(Adafruit_FONA *fona, OneWire *onewire, Adafruit_SSD1306 *display){
  _fona=fona;
  _onewire=onewire;
  _display=display;
}

float Monitor::GetVolts(){
  uint16_t reading=analogRead(5);
  //vbusVolts = ((float) reading*5.544)/1023.0;
  return ((float) reading*5.544)/1023.0;
}


uint8_t Monitor::GetNetworkStatus(byte statword){
  if (statword & 0x01){
    return _fona->getNetworkStatus();
  }
  else{
    return 0;
  }
}

uint8_t Monitor::GetSignal(byte statword){
  if (statword & 0x01){
    return _fona->getRSSI();
  }
  else{
    return 0;
  }
}

