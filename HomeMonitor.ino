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
//byte status=0b00000001; //Status word.  Bit0=PowerOn

OneWire  ds(ONEWIRE_PIN);

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

Adafruit_SSD1306 display(-1); //-1 because there is no reset pin

Monitor monitor(&fona,&ds,&display);

void setup() {
  delay(1000); //a delay is needed here for the display to initialize when recovering from power outage (non-reset). Not sure how long.
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  bitSet(monitor.status,DISPLAY_ON);
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

  monitor.SetupTemp();
  
  monitor.UpdateValues();
  time=millis();
  displaytime=time;
}

void loop() {
  //purge out old fona data regularly
  while (fona.available()) {
    Serial.write(fona.read());
  }

  //if Power is Off
  if bitRead(monitor.status,POWERSAVE_MODE){
    monitor.UpdateValues();
    monitor.UpdateStatus();
  }
  
  //if Power is On
  else{
    time=millis();
    if ((time-displaytime)>=DISPLAY_INTERVAL){
      //check temperature here
      monitor.UpdateValues(); //modify to only run when power screen is on.
      monitor.UpdateStatus();
      monitor.UpdateDisplay();
      displaytime+=DISPLAY_INTERVAL;
    }
  }
}








