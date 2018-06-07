
#include "Adafruit_FONA.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <OneWire.h>

//Define Pins
#define FONA_RX  9
#define FONA_TX  8
#define FONA_RST 4
#define FONA_RI  7
#define ONEWIRE_PIN 10

#include "PhoneNumber.h" //separate header file for phone number which defines PHONE_NUMBER.  You can comment this and define your own in the next line.
//#define PHONE_NUMBER "5555555555"

//Define Parameters
#define DISPLAY_INTERVAL  1000

//timer variables
unsigned int time;
unsigned int displaytime;

boolean PowerOn=true;

OneWire  ds(ONEWIRE_PIN);

SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

Adafruit_SSD1306 display(-1); //-1 because there is no reset pin

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
    display.display();
    while (1);
  }
  
  Serial.print(F("FONA> "));
  //fona.setGPRSNetworkSettings(F("wholesale"));

  ds.reset();
  ds.skip();
  ds.write(0x44);
  
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
    updatescreen();
    displaytime+=DISPLAY_INTERVAL;
  }
}

void updatescreen(){
  uint8_t x;
  uint8_t y;
  display.clearDisplay();
  display.setTextSize(1);
  
  display.setCursor(0,24);
  uint8_t n = fona.getNetworkStatus();
  if (n == 0) display.print(F("Not Registered"));
  if (n == 1) display.print(F("Registered"));
  if (n == 2) display.print(F("Searching"));
  if (n == 3) display.print(F("Denied"));
  if (n == 4) display.print(F("Unknown"));
  if (n == 5) display.print(F("Roaming"));

  n = fona.getRSSI();
  display.setCursor(0,0);
  display.print(F("RSSI:"));
  if (n>0){
    display.print("+");
  }
  display.print(n);

  uint16_t vbat;
  fona.getBattPercent(&vbat);
  if (vbat==100) x=103;
  if ((vbat>9) && (vbat<100)) x=109;
  if (vbat<=9) x=115;
  x-=12;
  display.setCursor(x,0);
  display.print(vbat);
  display.print("%");

  display.drawRect(118, 0, 10, 7, WHITE);
  display.drawRect(116,1,2,5,WHITE);
  display.drawRect(117,2,2,3,BLACK);
  if (vbat>=95) display.drawRect(117,2,2,3,WHITE);
  if (vbat>=85) display.drawRect(119,1,1,5,WHITE);
  if (vbat>=75) display.drawRect(120,1,1,5,WHITE);
  if (vbat>=66) display.drawRect(121,1,1,5,WHITE);
  if (vbat>=50) display.drawRect(122,1,1,5,WHITE);
  if (vbat>=33) display.drawRect(123,1,1,5,WHITE);
  if (vbat>=25) display.drawRect(124,1,1,5,WHITE);
  if (vbat>=15) display.drawRect(125,1,1,5,WHITE);
  if (vbat>=5) display.drawRect(126,1,1,5,WHITE);

  ds.reset();
  ds.skip();
  ds.write(0xBE);
  uint16_t raw=0;
  raw |= ds.read();
  raw |= ds.read()<<8;
  float temp = (float)raw / 16.0 * 1.8 + 32 ;
  display.setCursor(0,16);
  display.print(temp,1);
  display.print(" ");
  ds.reset();
  ds.skip();
  ds.write(0x44);

  int volt=analogRead(5);
  float voltage= ((float) volt*5.544)/1023.0;
  display.print(voltage,2);

  if (PowerOn==true && voltage<4.3){
    display.ssd1306_command(0xAE);
    PowerOn=false;
    fona.sendSMS(PHONE_NUMBER, "Power Lost");
  }
  if (PowerOn==false && voltage>=4.3){
    display.ssd1306_command(0xAF);
    PowerOn=true;
    fona.sendSMS(PHONE_NUMBER, "Power Restored");
  }
  
  display.display();
}
