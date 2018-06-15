
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
boolean PowerOn=true;
char status1[21]={0}; //Status Display Message Line 1
char status2[21]={0}; //Status Display Message Line 2


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
    //check temperature here
    updatescreen(); //modify to only run when power screen is on.
    displaytime+=DISPLAY_INTERVAL;
  }
}

void updatescreen(){
  /*define display variables*/
  float temperature;    //Temperature in F
  float vbusVolts;      //current voltage of Vbus
  char time[6]={0};     //Time to display
  uint16_t battery;     //Battery Percentage
  uint8_t signal;       //Signal Strength
  boolean connection;   //Connection status
  //status1,status2 declared earlier
  
  /*define input variables*/
  uint16_t reading;     //reading from analogRead and raw temperature reading
  char buffer[23];      //buffer to hold time string from Fona
  uint8_t n;            //holds rssi level, network status

  /*other variables*/
  uint8_t x;            //used to calculate x positions
  
  reading=analogRead(5);
  vbusVolts= ((float) reading*5.544)/1023.0;

  //triggered when power status changes state
  if (PowerOn==true && vbusVolts<4.3){
    //display.ssd1306_command(0xAE);
    PowerOn=false;
    //String("Power Restored").toCharArray(status1,20);
    memcpy(status1,"Power Lost",20);
    #ifdef PHONE_NUMBER
    fona.sendSMS(PHONE_NUMBER, "Power Lost");
    #endif
  }
  if (PowerOn==false && vbusVolts>=4.3){
    //display.ssd1306_command(0xAF);
    PowerOn=true;
    //String("Power Restored").toCharArray(status1,20);
    memcpy(status1,"Power Restored",20);
    #ifdef PHONE_NUMBER
    fona.sendSMS(PHONE_NUMBER, "Power Restored");
    #endif
  }

  display.clearDisplay();
  display.setTextSize(1);
  
  display.setCursor(0,0);
  connection = fona.getNetworkStatus();
  if (connection == 1){
    display.drawLine(0,4,1,6,WHITE);
    display.drawLine(1,6,4,0,WHITE);
  }
  if (connection == 5) display.print(F("R"));
  if (connection == 0 || connection == 2 || connection == 3 || connection == 4) display.print(F("X"));

  signal = fona.getRSSI();
  display.setCursor(12,0);
  if (signal>0) display.drawPixel(5,6,WHITE);
  if (signal>5) display.drawLine(7,6,8,4,WHITE);
  if (signal>10) display.drawLine(9,6,10,2,WHITE);
  if (signal>15) display.drawLine(11,6,12,0,WHITE);

  fona.getTime(buffer, 23);
  memcpy(time, buffer + 10, 5);
  display.setCursor(18,0);
  display.print(time);

  if (PowerOn==true){
    fona.getBattPercent(&battery);
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
    display.print(vbusVolts,2);
    display.print("V");
  }
  
  //read temp data from scratchpad
  ds.reset();
  ds.skip();
  ds.write(0xBE);
  reading=0;
  reading |= ds.read();
  reading |= ds.read()<<8;
  
  //start next reading
  ds.reset();
  ds.skip();
  ds.write(0x44);
  
  //calculate temperature in degrees F
  temperature = (float)reading / 16.0 * 1.8 + 32;
  display.setCursor(50,0);
  display.print(temperature,1);
  display.print((char)247);

  display.setCursor(0,16);
  display.print(status1);
  display.setCursor(0,24);
  display.print(status2);
  
  display.display();
}
