#include "Arduino.h"
#include "Monitor.h"

Monitor::Monitor(){
  var1=1;
}

float Monitor::GetVolts(){
  uint16_t reading=analogRead(5);
  //vbusVolts = ((float) reading*5.544)/1023.0;
  return ((float) reading*5.544)/1023.0;
}

