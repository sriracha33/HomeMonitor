/***************************************************
  This is an example for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963
  ----> http://www.adafruit.com/products/2468
  ----> http://www.adafruit.com/products/2542

  These cellular modules use TTL Serial to communicate, 2 pins are
  required to interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

/*
THIS CODE IS STILL IN PROGRESS!

Open up the serial console on the Arduino at 115200 baud to interact with FONA

Note that if you need to set a GPRS APN, username, and password scroll down to
the commented section below at the end of the setup() function.
*/
#include "Adafruit_FONA.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define FONA_RX  9
#define FONA_TX  8
#define FONA_RST 4
#define FONA_RI  7
#define OLED_RESET 4
Adafruit_SSD1306 display(-1);

// this is a large buffer for replies
char replybuffer[255];

//timer variables
unsigned int time;
unsigned int displaytime;
#define DISPLAY_INTERVAL  1000

#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

void setup() {
  delay(1000);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(12,0);
  display.println(F("Booting..."));
  display.display();

  if (!Serial) delay(2000);
  
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

  updatescreen();
  time=millis();
  displaytime=time;
}

void loop() {
  while (fona.available()) {
    Serial.write(fona.read());
  }

  if (Serial.available()){
    char command = Serial.read();
    Serial.println(command);
    docommand(command);
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
  
  display.display();
}

void docommand(char command){
  switch (command) {

    case 'a': {
        // read the ADC
        uint16_t adc;
        if (! fona.getADCVoltage(&adc)) {
          Serial.println(F("Failed to read ADC"));
        } else {
          Serial.print(F("ADC = ")); Serial.print(adc); Serial.println(F(" mV"));
        }
        break;
      }

    case 'b': {
        // read the battery voltage and percentage
        uint16_t vbat;
        if (! fona.getBattVoltage(&vbat)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VBat = ")); Serial.print(vbat); Serial.println(F(" mV"));
        }


        if (! fona.getBattPercent(&vbat)) {
          Serial.println(F("Failed to read Batt"));
        } else {
          Serial.print(F("VPct = ")); Serial.print(vbat); Serial.println(F("%"));
        }

        break;
      }

    case 'i': {
        // read the RSSI
        uint8_t n = fona.getRSSI();
        int8_t r;

        Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
        if (n == 0) r = -115;
        if (n == 1) r = -111;
        if (n == 31) r = -52;
        if ((n >= 2) && (n <= 30)) {
          r = map(n, 2, 30, -110, -54);
        }
        Serial.print(r); Serial.println(F(" dBm"));

        break;
      }

    case 'n': {
        // read the network/cellular status
        uint8_t n = fona.getNetworkStatus();
        Serial.print(F("Network status "));
        Serial.print(n);
        Serial.print(F(": "));
        if (n == 0) Serial.println(F("Not registered"));
        if (n == 1) Serial.println(F("Registered (home)"));
        if (n == 2) Serial.println(F("Not registered (searching)"));
        if (n == 3) Serial.println(F("Denied"));
        if (n == 4) Serial.println(F("Unknown"));
        if (n == 5) Serial.println(F("Registered roaming"));
        break;
      }

    /*** SMS ***/

    case 'N': {
        // read the number of SMS's!
        int8_t smsnum = fona.getNumSMS();
        if (smsnum < 0) {
          Serial.println(F("Could not read # SMS"));
        } else {
          Serial.print(smsnum);
          Serial.println(F(" SMS's on SIM card!"));
        }
        break;
      }
    case 'r': {
        // read an SMS
        flushSerial();
        Serial.print(F("Read #"));
        uint8_t smsn = readnumber();
        Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);

        // Retrieve SMS sender address/phone number.
        if (! fona.getSMSSender(smsn, replybuffer, 250)) {
          Serial.println("Failed!");
          break;
        }
        Serial.print(F("FROM: ")); Serial.println(replybuffer);

        // Retrieve SMS value.
        uint16_t smslen;
        if (! fona.readSMS(smsn, replybuffer, 250, &smslen)) { // pass in buffer and max len!
          Serial.println("Failed!");
          break;
        }
        Serial.print(F("***** SMS #")); Serial.print(smsn);
        Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
        Serial.println(replybuffer);
        Serial.println(F("*****"));

        break;
      }
    case 'R': {
        // read all SMS
        int8_t smsnum = fona.getNumSMS();
        uint16_t smslen;
        int8_t smsn;
        smsn = 1;  // 1 indexed

        for ( ; smsn <= smsnum; smsn++) {
          Serial.print(F("\n\rReading SMS #")); Serial.println(smsn);
          if (!fona.readSMS(smsn, replybuffer, 250, &smslen)) {  // pass in buffer and max len!
            Serial.println(F("Failed!"));
            break;
          }
          // if the length is zero, its a special case where the index number is higher
          // so increase the max we'll look at!
          if (smslen == 0) {
            Serial.println(F("[empty slot]"));
            smsnum++;
            continue;
          }

          Serial.print(F("***** SMS #")); Serial.print(smsn);
          Serial.print(" ("); Serial.print(smslen); Serial.println(F(") bytes *****"));
          Serial.println(replybuffer);
          Serial.println(F("*****"));
        }
        break;
      }

    case 'd': {
        // delete an SMS
        flushSerial();
        Serial.print(F("Delete #"));
        uint8_t smsn = readnumber();

        Serial.print(F("\n\rDeleting SMS #")); Serial.println(smsn);
        if (fona.deleteSMS(smsn)) {
          Serial.println(F("OK!"));
        } else {
          Serial.println(F("Couldn't delete"));
        }
        break;
      }

    case 's': {
        // send an SMS!
        char sendto[21], message[141];
        flushSerial();
        Serial.print(F("Send to #"));
        readline(sendto, 20);
        Serial.println(sendto);
        Serial.print(F("Type out one-line message (140 char): "));
        readline(message, 140);
        Serial.println(message);
        if (!fona.sendSMS(sendto, message)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("Sent!"));
        }

        break;
      }

    /*** Time ***/

    case 'y': {
        // enable network time sync
        if (!fona.enableNetworkTimeSync(true))
          Serial.println(F("Failed to enable"));
        break;
      }

    case 'Y': {
        // enable NTP time sync
        if (!fona.enableNTPTimeSync(true, F("pool.ntp.org")))
          Serial.println(F("Failed to enable"));
        break;
      }

    case 't': {
        // read the time
        char buffer[23];

        fona.getTime(buffer, 23);  // make sure replybuffer is at least 23 bytes!
        Serial.print(F("Time = ")); Serial.println(buffer);
        break;
    }
    /*****************************************/

    case 'S': {
        Serial.println(F("Creating SERIAL TUBE"));
        char inbyte;
        while (1) {
          while (Serial.available()) {
            delay(1);
            inbyte=Serial.read();
            if (inbyte!='~') fona.write(inbyte);
          }
          if (inbyte=='~'){
            Serial.println(F("Closing SERIAL TUBE"));
            break;
          }
          if (fona.available()) {
            Serial.write(fona.read());
          }
        }
        break;
      }

    default: {
        Serial.println(F("Unknown command"));
        break;
      }
  }
  // flush input
  flushSerial();
  Serial.print(F("FONA> "));
}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}

char readBlocking() {
  while (!Serial.available());
  return Serial.read();
}
uint16_t readnumber() {
  uint16_t x = 0;
  char c;
  while (! isdigit(c = readBlocking())) {
    //Serial.print(c);
  }
  Serial.print(c);
  x = c - '0';
  while (isdigit(c = readBlocking())) {
    Serial.print(c);
    x *= 10;
    x += c - '0';
  }
  return x;
}

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
