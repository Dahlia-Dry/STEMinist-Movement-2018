/*@author Dahlia Dry
 * @version 1
 * Mode 1: Display temp, humidity, heat index in F, C
 * Mode 2: Live graph temperature
 * Mode 3: Live graph humidity
 * last modified September 2018
 */

//LCD display setup
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#define TFT_CS     10
#define TFT_RST    9 
#define TFT_DC     8
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

//DHT11 setup-comment this section out (put // before each line) 
//if you're using a DHT22 instead
#define DHTTYPE DHT11

//Uncomment this section if you're using the DHT22 instead
//#define DHTTYPE DHT22

//general DHT setup
#include "DHT.h"
#define DHTPIN 2
#include <avr/wdt.h>
DHT dht(DHTPIN, DHTTYPE);

//declare additional vars
uint8_t rotation = 3;
const int buttonPin = 3;
int buttonState = 0;
int lastButtonState = 0;
int buttonPushCounter = 0;
long time = 0;
long debounce = 200;
int xPos = 5;
int graphHeightPrevious = 0;
float humid_init = dht.readHumidity();

//tell the screen to power on, start the clock, get mode 1 ready
void setup() {
  tft.initR(INITR_BLACKTAB);
  pinMode(buttonPin, INPUT);
  uint16_t time = millis();
  tft.setRotation(rotation);
  setupmode1();
}

//"draw" text on the scrren - takes args: 
//text (what you want to write),
//color (what color do you want what you're writing)
//x0,y0 (the coordinates of the top left hand corner of your text)
void drawtext(String text, uint16_t color, uint16_t x0, uint16_t y0){
  tft.setCursor(x0,y0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

//initial setup to display current temp,humidity,heat index readings
//drawtext function to tell screen what to display (see above)
void setupmode1(){ 
  tft.fillScreen(ST7735_BLACK);
  drawtext("WEATHER DATA", ST7735_WHITE,0,0);
  drawtext("Temperature (F):", ST7735_WHITE,0,13);
  drawtext("Temperature (C):", ST7735_WHITE, 0,26);
  drawtext("Humidity (%):", ST7735_WHITE, 0,39);
  drawtext("Heat Index (F):", ST7735_WHITE, 0, 52);
  drawtext("Heat Index (C):", ST7735_WHITE, 0,65);
  
}

//loop that refreshes mode 1 every 2 secnods
//asks sensor what new temp, humidity readings are every 2 seconds
void executemode1(){
  tft.fillRect(95, 13, 60,65, ST7735_BLACK);
  float tempf = dht.readTemperature(true);
  float tempc = dht.readTemperature();
  float humid = dht.readHumidity();
  float heatf = dht.computeHeatIndex(tempf, humid);
  float heatc = dht.computeHeatIndex(tempc,humid,false);
  drawtext(String(tempf), ST7735_WHITE,95,13);
  drawtext(String(tempc), ST7735_WHITE, 95,26);
  drawtext(String(humid), ST7735_WHITE, 95,39);
  drawtext(String(heatf), ST7735_WHITE, 95, 52);
  drawtext(String(heatc), ST7735_WHITE, 95, 65);
}

//setup live graph of temperature by setting constant label and initial (x,y)
void setupmode2(){
  tft.fillScreen(ST7735_WHITE);
  drawtext("Temperature (F):",ST7735_BLACK, 0,0);
  xPos = 0;
  graphHeightPrevious = 0;
}

//setup live graph of humidity by setting constant label and inital (x,y)
void setupmode3(){
  tft.fillScreen(ST7735_WHITE);
  drawtext("Humidity (%):",ST7735_BLACK, 0,0);
  xPos = 0;
  graphHeightPrevious = 0;
}

//temperature graph loop
//loop that plots a new point on the graph every 5000 ms
//drawCircle plots new point, drawLine connects new point with previous point
//reset graph if the x position exceeds the width of the screen
//each point is 5 pixels apart in the x direction, so the graph will hold 32 points
void executemode2(){
  tft.fillRect(95,0,40,7, ST7735_WHITE);
  float tempf = dht.readTemperature(true);
  drawtext(String(tempf),ST7735_BLACK,95,0);
  int graphHeight = map(tempf, 100, 50, 10, 128);
  tft.drawCircle(xPos,graphHeight,2, ST7735_BLUE);
  if(xPos > 5){
    tft.drawLine(xPos-5,graphHeightPrevious,xPos,graphHeight,ST7735_RED);
  }
  graphHeightPrevious = graphHeight;
  if(xPos >= 160){
    xPos = 5;
    tft.fillScreen(ST7735_WHITE);
    drawtext("Temperature (F):",ST7735_BLACK, 0,0);
  }
  else{
    xPos += 5;
  }
}

//humidity graph loop
//loop that plots a new point on the graph every 5000 ms
//drawCircle plots new point, drawLine connects new point with previous point
//reset graph if the x position exceeds the width of the screen
//each point is 5 pixels apart in the x direction, so the graph will hold 32 points
void executemode3(){
  tft.fillRect(95,0,40,7, ST7735_WHITE);
  float humid = dht.readHumidity();
  drawtext(String(humid),ST7735_BLACK,95,0);
  int graphHeight = map(humid, 100, 0, 10, 128);
  tft.drawCircle(xPos,graphHeight,2, ST7735_BLUE);
  if(xPos > 5){
    tft.drawLine(xPos-5,graphHeightPrevious,xPos,graphHeight,ST7735_RED);
  }
  graphHeightPrevious = graphHeight;
  if(xPos >= 160){
    xPos = 5;
    tft.fillScreen(ST7735_WHITE);
    drawtext("Humidity (%):",ST7735_BLACK, 0,0);
  }
  else{
    xPos += 5;
  }
}

//main loop - tells atmega which mode to loop through depending on button pushes
//works by counting up button pushes- if divisible by 3, execute mode 1, etc
//eg. 30 / 3 has remainder 0, so execute mode 1
//delay(5000) tells atmega to update display and readings every 5000 milliseconds
void loop() {
  buttonState = digitalRead(buttonPin);
  if(buttonState == HIGH && millis() - time > debounce){
    //indicates off -> on
    buttonPushCounter++; //increment count of push number
    time = millis();
    if(buttonPushCounter % 3 == 0){
      setupmode1();
    }
    else if(buttonPushCounter % 3 == 1){
      setupmode2();
    }
    else if(buttonPushCounter % 3 == 2){
      setupmode3();
    }
  }

  if(buttonPushCounter % 3 == 0){
    executemode1();
  }
  else if(buttonPushCounter % 3 == 1){
    executemode2();
  }
  else if(buttonPushCounter % 3 == 2){
    executemode3();
  }
  delay(5000);
}
