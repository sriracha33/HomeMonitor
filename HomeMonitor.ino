#include <Adafruit_FONA.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include "Monitor.h"

//Define Pins
#define FONA_RX  9
#define FONA_TX  8
#define FONA_RST 4
#define FONA_RI  7
#define ONEWIRE_PIN 10
//pin for button 11? Use pcint

/*  Separate header file for phone number which defines PHONE_NUMBER.  You can comment this and define your own in the next line.
    Leave both commented out to prevent any SMS from being sent */
//#include "PhoneNumber.h" 
//#define PHONE_NUMBER "5555555555"

//Define Parameters
#define DISPLAY_INTERVAL  1000

//timer variables
unsigned int time;
unsigned int displaytime;

/*state variable that need to be maintained across main loops*/
byte status=0b00000001; //Status word.  Bit0=PowerOn
char status1[18]={0};   //Status Display Message Line 1 (Timestamp)
char status2[21]={0};   //Status Display Message Line 2

OneWire  ds(ONEWIRE_PIN);

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

Adafruit_SSD1306 display(-1); //-1 because there is no reset pin

Monitor monitor(&fona,&ds,&display);

void setup() {
  delay(1000); //a delay is needed here for the display to initialize when recovering from power outage (non-reset). Not sure how long.
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(12,0);
  display.println(F("Booting..."));
  display.display();

  if (!Serial) delay(2000); //give you a chance to open the serial monitor before proceeding.
  Serial.begin(115200);
  Serial.println(F("Initializing...."));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }

  SetupTemp();
  //Serial.println(monitor.GetNetworkStatus(0x01));
  
  updatescreen();
  time=millis();
  displaytime=time;
}

void loop() {
  while (fona.available()) {
    Serial.write(fona.read());
  }

  time=millis();
  if ((time-displaytime)>=DISPLAY_INTERVAL){
    //check temperature here
    updatescreen(); //modify to only run when power screen is on.
    displaytime+=DISPLAY_INTERVAL;
  }
}

void updatescreen(){
  /*define display variables*/
  int16_t temperature;  //Temperature in F
  char currentTime[6];  //Time to display
  char dateTime[18];    //Date and Time stamp
  uint8_t battery;     //Battery Percentage
  //uint8_t signal;       //Signal Strength
  //uint8_t connection;   //Connection status
  //status1,status2 declared earlier

  /*other variables*/
  uint8_t x;            //used to calculate x positions for display
  
  //vbusVolts=GetVolts();
  monitor.vbusVolts=monitor.GetVolts();
  
  //triggered when power status changes state
  if ((status & 0x01) && monitor.vbusVolts<4.3){
    //display.ssd1306_command(0xAE);
    GetTime(currentTime,dateTime,status);
    status&=0xFE; //set power ok status to off
    memcpy(status1,dateTime,18);
    memcpy(status2,"Power Lost",20);
    GetTime(currentTime,dateTime,status);
    #ifdef PHONE_NUMBER
    fona.sendSMS(PHONE_NUMBER, "Power Lost");
    #endif
  }
  else if (!(status & 0x01) && monitor.vbusVolts>=4.3){
    //display.ssd1306_command(0xAF);
    status|=0x01; //set power ok status to off
    GetTime(currentTime,dateTime,status);
    memcpy(status1,dateTime,18);
    memcpy(status2,"Power Restored",20);
    #ifdef PHONE_NUMBER
    fona.sendSMS(PHONE_NUMBER, "Power Restored");
    #endif
  }
  else{
    GetTime(currentTime,dateTime,status);
  }
  
  temperature = GetTemp();
  monitor.connection = monitor.GetNetworkStatus(status);
  monitor.signal = monitor.GetSignal(status);
  battery=GetBattery(status);
  
  /*Display Code*/
  
  //setup display
  display.clearDisplay();
  display.setTextSize(1);
  
  //Display Time
  display.setCursor(19,0);
  display.print(currentTime);

  //Display Connection Status
  display.setCursor(0,0);
  if (monitor.connection == 1){
    display.drawLine(0,4,1,6,WHITE);
    display.drawLine(1,6,4,0,WHITE);
  }
  if (monitor.connection == 5) display.print(F("R"));
  if (monitor.connection == 0 || monitor.connection == 2 || monitor.connection == 3 || monitor.connection == 4) display.print(F("X"));

  //Display Signal Strength
  display.setCursor(12,0);
  if (monitor.signal>0) display.drawPixel(5,6,WHITE);
  if (monitor.signal>5) display.drawLine(7,6,7,4,WHITE);
  if (monitor.signal>10) display.drawLine(9,6,9,2,WHITE);
  if (monitor.signal>15) display.drawLine(11,6,11,0,WHITE);

  //Display Battery Status (from Fona or ADC);
  if (battery){
    if (battery==100) x=103;
    if ((battery>9) && (battery<100)) x=109;
    if (battery<=9) x=115;
    x-=12;
    display.setCursor(x,0);
    display.print(battery);
    display.print("%");
  
    display.drawRect(118, 0, 10, 7, WHITE);
    display.drawRect(116,1,2,5,WHITE);
    display.drawRect(117,2,2,3,BLACK);
    if (battery>=95) display.drawRect(117,2,2,3,WHITE);
    if (battery>=85) display.drawRect(119,1,1,5,WHITE);
    if (battery>=75) display.drawRect(120,1,1,5,WHITE);
    if (battery>=66) display.drawRect(121,1,1,5,WHITE);
    if (battery>=50) display.drawRect(122,1,1,5,WHITE);
    if (battery>=33) display.drawRect(123,1,1,5,WHITE);
    if (battery>=25) display.drawRect(124,1,1,5,WHITE);
    if (battery>=15) display.drawRect(125,1,1,5,WHITE);
    if (battery>=5) display.drawRect(126,1,1,5,WHITE);
  }
  else{
    display.setCursor(97,0);
    display.print(monitor.vbusVolts,2);
    display.print("V");
  }

  //Display Temperature
  x=64;
  if (temperature>=100 || temperature<=-10 ) x-=6;
  if (temperature<=9 && temperature>=0) x+=6;
  display.setCursor(x,0);
  display.print(temperature);
  display.print((char)247);  //degrees

  //Display Event
  display.setCursor(12,16);
  display.print(status1);
  display.setCursor(0,24);
  display.print(status2);

  //Update Display
  display.display();
}

void SetupTemp(){
  uint8_t config=0;
  //check resolution
  ds.reset();
  ds.skip();
  ds.write(0xBE); //Read Scratchpad
  ds.read();ds.read();ds.read();ds.read(); //ignore first 4 bytes.
  config=ds.read();

  if (config!=0x1F){
    //Set Configuration register. Resolution to 9 bits.
    ds.reset();
    ds.skip();
    ds.write(0x4E); //write scratchpad
    ds.write(0x64); //High Limit. Set to 100. Don't really care
    ds.write(0x00); //Low Limit. Set to 0. Don't really care
    ds.write(0x1F); //Resolution to 9 bits.
    ds.reset();     //Reset required to make changes active.
    ds.skip();
    ds.write(0x48); //Copy Scratchpad (to EEPROM);
    while (!ds.read_bit()){}; //wait for save to complete  
  }
  
  ds.reset();
  ds.skip();
  ds.write(0x44,0);
}

int GetTemp(){
  int16_t reading=0;

  //start next reading
  ds.reset();
  ds.skip();
  ds.write(0x44);

  while (!ds.read_bit()); //wait for conversion. About 70ms for 9 bit.
  
  //read temp data from scratchpad
  ds.reset();
  ds.skip();
  ds.write(0xBE);
  reading |= ds.read();
  reading |= ds.read()<<8;

  reading=reading>>3; //drop useless bits. leave one for rounding
  
  //convert temp to degree F
  reading=reading*(9);
  reading=reading+(160<<1);
  reading=reading/(5);
  reading=reading+reading%2; //round values up in magnitude
  reading=reading>>1; 
 
  return reading;
}

float GetVolts(){
  uint16_t reading=analogRead(5);
  return ((float) reading*5.544)/1023.0;
}

void GetTime(char currentTime[],char dateTime[], byte statword){
  memset(currentTime,0,6);
  memset(dateTime,0,18);
  if (statword & 0x01){
    char datetime[23];
    fona.getTime(datetime, 23);
    memcpy(currentTime, datetime + 10, 5);
    memcpy(dateTime,datetime+1,17);
  }
}

uint8_t GetBattery(byte statword){
  if (statword & 0x01){
    uint16_t batt;
    fona.getBattPercent(&batt);  
    return batt;
  }
  else{
    return 0;
  }
}




