#include "Arduino.h"
#include "Monitor.h"

Monitor::Monitor(Adafruit_FONA *fona, OneWire *onewire, Adafruit_SSD1306 *display) {
  _fona = fona;
  _onewire = onewire;
  _display = display;
  
}

float Monitor::GetVolts() {
  uint16_t reading = analogRead(5);
  return ((float) reading * 5.544) / 1023.0;
}

uint8_t Monitor::GetBattery(){
  if (!bitRead(status,LOW_VOLTS)){
    uint16_t batt;
    _fona->getBattPercent(&batt);  
    return batt;
  }
  else{
    return 0;
  }
}

uint8_t Monitor::GetNetworkStatus() {
  if (!bitRead(status,LOW_VOLTS)) {
    return _fona->getNetworkStatus();
  }
  else {
    return 0;
  }
}

uint8_t Monitor::GetSignal() {
  if (!bitRead(status,LOW_VOLTS)) {
    return _fona->getRSSI();
  }
  else {
    return 0;
  }
}

void Monitor::GetTime(){
  memset(currentTime,0,6);
  memset(dateTime,0,18);
  if (!bitRead(status,LOW_VOLTS)){
    char datetime[23];
    _fona->getTime(datetime, 23);
    memcpy(currentTime, datetime + 10, 5);
    memcpy(dateTime,datetime+1,17);
  }
}

void Monitor::SetupTemp() {
  uint8_t config = 0;
  //check resolution
  _onewire->reset();
  _onewire->skip();
  _onewire->write(0xBE); //Read Scratchpad
  _onewire->read(); _onewire->read(); _onewire->read(); _onewire->read(); //ignore first 4 bytes.
  config = _onewire->read();

  if (config != 0x1F) {
    //Set Configuration register. Resolution to 9 bits.
    _onewire->reset();
    _onewire->skip();
    _onewire->write(0x4E); //write scratchpad
    _onewire->write(0x64); //High Limit. Set to 100. Don't really care
    _onewire->write(0x00); //Low Limit. Set to 0. Don't really care
    _onewire->write(0x1F); //Resolution to 9 bits.
    _onewire->reset();     //Reset required to make changes active.
    _onewire->skip();
    _onewire->write(0x48); //Copy Scratchpad (to EEPROM);
    while (!_onewire->read_bit()) {}; //wait for save to complete
  }

  _onewire->reset();
  _onewire->skip();
  _onewire->write(0x44, 0);
}

int Monitor::GetTemp() {
  int16_t reading = 0;

  //start next reading
  _onewire->reset();
  _onewire->skip();
  _onewire->write(0x44);

  while (!_onewire->read_bit()); //wait for conversion. About 70ms for 9 bit.

  //read temp data from scratchpad
  _onewire->reset();
  _onewire->skip();
  _onewire->write(0xBE);
  reading |= _onewire->read();
  reading |= _onewire->read() << 8;

  reading = reading >> 3; //drop useless bits. leave one for rounding

  //convert temp to degree F
  reading = reading * (9);
  reading = reading + (160 << 1);
  reading = reading / (5);
  reading = reading + reading % 2; //round values up in magnitude
  reading = reading >> 1;

  return reading;
}

void Monitor::UpdateValues(){
  vbusVolts = GetVolts();
  temperature = GetTemp();
  connection = GetNetworkStatus();
  signal = GetSignal();
  battery = GetBattery();
  GetTime();
  
  //triggered when power status changes state
  if (!bitRead(status,LOW_VOLTS) && vbusVolts<4.3){
    //display.ssd1306_command(0xAE);
    bitSet(status,LOW_VOLTS);//set power off status to on
    memcpy(eventTime,dateTime,18);
    memcpy(eventText,"Power Lost",20);
    #ifdef PHONE_NUMBER
    fona.sendSMS(PHONE_NUMBER, "Power Lost");
    #endif
  }
  else if (bitRead(status,LOW_VOLTS) && vbusVolts>=4.3){
    //display.ssd1306_command(0xAF);
    bitClear(status,LOW_VOLTS); //set power off status to off
    memcpy(eventTime,dateTime,18);
    memcpy(eventText,"Power Restored",20);
    #ifdef PHONE_NUMBER
    fona.sendSMS(PHONE_NUMBER, "Power Restored");
    #endif
  }
}

void Monitor::UpdateStatus(){
  byte lastStatus = status;
  
  /*start with alarms*/
  //Detect if AC power is lost
  //if (vbusVolts<VOLT_LIMIT && !bitRead(status,LOW_VOLTS)){bitSet(status,LOW_VOLTS);}
  //else if (vbusVolts>VOLT_LIMIT && bitRead(status,LOW_VOLTS)){bitClear(status,LOW_VOLTS);}
  
  //Detect if Battery level is low. Use 0.2v deadband to prevent refiring.
  if (vbusVolts<BATTERY_LIMIT && !bitRead(status,LOW_BATTERY)){bitSet(status,LOW_BATTERY);}
  else if (vbusVolts>BATTERY_LIMIT+0.2 && bitRead(status,LOW_BATTERY)){bitClear(status,LOW_BATTERY);}
  
  //Detect if temperature is below limit
  if (temperature<TEMP_LIMIT  && !bitRead(status,LOW_TEMP)){bitSet(status,LOW_TEMP);Serial.println("TEMP LOW");}
  else if (temperature>TEMP_LIMIT+2  && bitRead(status,LOW_TEMP)){bitClear(status,LOW_TEMP);Serial.println("TEMP NORMAL");}

  

  
  
}

void Monitor::UpdateDisplay(){
  if (!bitRead(status,DISPLAY_ON)) return;
  
  uint8_t x;  //used to calculate x positions.
  
  //setup display
  _display->clearDisplay();
  
  //Display Time
  _display->setCursor(19,0);
  _display->print(currentTime);

  //Display Connection Status
  _display->setCursor(0,0);
  if (connection == 1){
    _display->drawLine(0,4,1,6,WHITE);
    _display->drawLine(1,6,4,0,WHITE);
  }
  if (connection == 5) _display->print(F("R"));
  if (connection == 0 || connection == 2 || connection == 3 || connection == 4) _display->print(F("X"));

  //Display Signal Strength
  _display->setCursor(12,0);
  if (signal>0) _display->drawPixel(5,6,WHITE);
  if (signal>5) _display->drawLine(7,6,7,4,WHITE);
  if (signal>10) _display->drawLine(9,6,9,2,WHITE);
  if (signal>15) _display->drawLine(11,6,11,0,WHITE);

  //Display Battery Status (from Fona or ADC);
  if (battery){
    if (battery==100) x=91;
    if ((battery>9) && (battery<100)) x=97;
    if (battery<=9) x=13;
    //x-=12;
    _display->setCursor(x,0);
    _display->print(battery);
    _display->print("%");
  
    _display->drawRect(118, 0, 10, 7, WHITE);
    _display->drawRect(116,1,2,5,WHITE);
    _display->drawRect(117,2,2,3,BLACK);
    if (battery>=95) _display->drawRect(117,2,2,3,WHITE);
    if (battery>=85) _display->drawRect(119,1,1,5,WHITE);
    if (battery>=75) _display->drawRect(120,1,1,5,WHITE);
    if (battery>=66) _display->drawRect(121,1,1,5,WHITE);
    if (battery>=50) _display->drawRect(122,1,1,5,WHITE);
    if (battery>=33) _display->drawRect(123,1,1,5,WHITE);
    if (battery>=25) _display->drawRect(124,1,1,5,WHITE);
    if (battery>=15) _display->drawRect(125,1,1,5,WHITE);
    if (battery>=5) _display->drawRect(126,1,1,5,WHITE);
  }
  else{
    _display->setCursor(97,0);
    _display->print(vbusVolts,2);
    _display->print("V");
  }

  //Display Temperature
  x=64;
  if (temperature>=100 || temperature<=-10 ) x-=6;
  if (temperature<=9 && temperature>=0) x+=6;
  _display->setCursor(x,0);
  _display->print(temperature);
  _display->print((char)247);  //degrees

  //Display Event
  _display->setCursor(12,16);
  _display->print(eventTime);
  _display->setCursor(0,24);
  _display->print(eventText);

  //Update Display
  _display->display();
}

