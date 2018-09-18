/* @author Dahlia Dry
 * @version 2- added extensions and upgrade, streamlined structure
 * Mode 1 (Default): sequential flashing lights
 * Mode 2 (DisplayTemp): live display temperature
 * Mode 3 (DisplayHumidity): live display humidity
 * Mode 4 (LightMode): photoresistor to control led "bar graph"
 * Mode 5 (SoundLightMode): piezo buzzer song with animated lights
 * last modified September 2018
 */

//initial setup of dht11 sensor, just declaring some variables
#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
#include <avr/wdt.h>
DHT dht(DHTPIN, DHTTYPE);

//Frequencies of notes for SoundLightMode (mode 5)
#define NOTE_E6  1319
#define NOTE_G6  1568
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_D7  2349
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_G7  3136
#define NOTE_A7  3520
#define NOTE_B7  3951

//DECLARE ALL CONSTANTS AND INITIAL VALUES (there's a lot)
const int ledCount = 10; //number of LEDs on bar graph
const int buttonPin = 13; //digital pin 13 on atmega, the pin the button is on
const int photoPin = 0; //analog pin 0 on atmega
const boolean LED_OFF = LOW; //low state turns LED off
const boolean LED_ON = HIGH; //high state turns LED on
int photoValue = 0; //initial value for photoresistor
int ledLevel = 0; //how many LEDs turned on
int buttonState = 0; //1 = high, low = 0
int buttonPushCounter = 0; //how many times has button been pushed
int lastButtonState = 0; //0 or 1, track if button state has changed
int ledPins[] = {3,4,5,6,7,8,9,10,11,12}; //digital pins on atmega
long time = 0; //init value for time
long debounce = 200; //a thing that makes sure your button readings aren't wonky
int speakerPin = A1; //analog pin 1 on atmega, where the piezo is
int note_names[] = {NOTE_E6, NOTE_G6, NOTE_A6, NOTE_B6, NOTE_C7, 
  NOTE_D7, NOTE_E7, NOTE_F7, NOTE_G7, NOTE_B7}; //names of included notes
boolean extended = false; //should atmega include modes 4,5 in loop?

void software_reset(){ //just a safety to make sure everything reboots properly
  wdt_reset();
  wdt_enable(WDTO_250MS);
  exit(1);
}

//open up serial monitor in case you're using arduino and want feedback on computer screen
//initialize LED and button pins
//if resistor is connected to analog pin 5, 
//the atmega will be able to tell you're in extended mode
void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  for(int thisLed = 0; thisLed < ledCount; thisLed ++){
    pinMode(ledPins[thisLed],OUTPUT);
  }
  dht.begin();
  int test = analogRead(A5);
  if(test == 0){
    extended = true;
  }
}

//mode 1
//sequential flashing lights
//turn light on, delay 500 milliseconds, turn light off
void Default() {
  for(int thisLed = 0; thisLed < ledCount; thisLed++){
      digitalWrite(ledPins[thisLed], HIGH);
      delay(500);
      digitalWrite(ledPins[thisLed], LOW);
    }
}

//this is the thing you see when you change btwn modes
//default, but faster - only 20 ms delay btwn flashes
void Quick(){
  for(int thisLed = 0; thisLed < ledCount; thisLed++){
      digitalWrite(ledPins[thisLed], HIGH);
      delay(20);
      digitalWrite(ledPins[thisLed], LOW);
    }
}

//mode 2
//update temperature readings every 2000 ms
//map temp reading to number of LEDs to light up 
//[50,100] -> [0,10]
void DisplayTemp() { //in Fahrenheit  //button option 1
  float t = dht.readTemperature(true); //tell the sensor to give fahrenheit value
  // Check if any reads failed and exit early 
  if (isnan(t)) {
    return;
  }
  for(int thisLed = 0; thisLed < ledCount; thisLed++){
      digitalWrite(ledPins[thisLed], LOW);
    }
  int thisLed = 0;
  int testTemp = 50;
  while(t > testTemp && thisLed < 10){
      digitalWrite(ledPins[thisLed], HIGH);
      thisLed ++;
      testTemp += 5;
    }
}

//mode 3
//update humidity readings every 2000 ms
//map % humidity to number of LEDs to light up
//[0,100] -> [0,10]
void DisplayHumidity(){
  float h = dht.readHumidity();
  Serial.println(h);
  //check if any reads failed and exit early
  if(isnan(h)){
    return;
  }
  for(int thisLed = 0; thisLed < ledCount; thisLed++){
      digitalWrite(ledPins[thisLed], LOW); //turn LED off
    }
  int thisLed = 0;
  int testTemp = 0;
  while(h > testTemp && thisLed < 10){
      digitalWrite(ledPins[thisLed], HIGH); //turn LED on
      thisLed ++;
      testTemp += 10;
    }
}

//mode 4
//map photoresistor value to number of LEDs lit up
//[0,700] -> [0,10]
void LightMode(){ //Extension Project 1  
  photoValue = analogRead(photoPin);
  ledLevel = map(photoValue,0,700,0,ledCount); 
  for(int led = 0; led < ledCount; led ++){
    if(led < ledLevel){
      digitalWrite(ledPins[led], LED_ON);//LED on
    }
    else{
      digitalWrite(ledPins[led], LED_OFF);//LED off
    }
  }
}

//mode 5
//piezo buzzer super mario theme with dancing lights
//the lights that flash correspond to notes
void SoundLightMode(){
  //melody- an array of notes to play. 0 = rest.
  int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0
  };
  
  //tempo- how long each note should last.
  int tempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  };
 
  int size = sizeof(melody) / sizeof(int); //measure of length of song
  for (int thisNote = 0; thisNote < size; thisNote++){
    int noteDuration = 1000 / tempo[thisNote]; //how long the buzzer should buzz each note
    tone(speakerPin, melody[thisNote], noteDuration); 
    //tone tells the buzzer at speakerPin (atmega pin 13) to oscillate 
    //at chosen frequency melody[thisNote] 
    //for noteDuration milliseconds

    //next, tell the lights to light up depending on what notes is played
    //AS6 and A7 were extra so I had them group with other notes
    for(int i = 0; i < 11; i ++){
      if(note_names[i] == melody[thisNote]){
        digitalWrite(ledPins[i], HIGH);
      }
      else if(melody[thisNote] == NOTE_AS6){
        digitalWrite(ledPins[2], HIGH);
      }
      else if (melody[thisNote] == NOTE_A7){
        digitalWrite(ledPins[9], HIGH);
      }
    }
    int pauseBetweenNotes = noteDuration * 1.3;
    delay(pauseBetweenNotes);
    //make sure all the LEDs turn back off after each flash
    for(int i = 0; i < ledCount; i ++){
      digitalWrite(ledPins[i], LOW);
    }
  }
}

//main loop- executes one of the modes depending on # of button presses and if extended = True
void loop() {
  if(extended){ //test if resistor connects analog pin 5 on atmega to negative terminal
    //indicates that we should loop through modes 1,2,3,4,5 instead of just 1,2,3
    buttonState = digitalRead(buttonPin);
    //check if the button is being pushed
    if(buttonState == HIGH && millis() - time > debounce){
      //indicates off -> on
      buttonPushCounter++; //increment count of push number
      time = millis();
      Quick(); //the visual mode change indicator- really quick light flash
      //% is called modulus- it's the remainder when you divide something
      //eg. 50 /5 = 10 remainder 0, so 50 % 5 = 0, and mode 1 would execute
      if(buttonPushCounter % 5 == 0){
        digitalWrite(ledPins[0], LED_ON); //the visual mode indicator
      }
      else if(buttonPushCounter % 5 == 1){
        digitalWrite(ledPins[1], LED_ON);//the visual mode indicator
      }
      else if(buttonPushCounter % 5 == 2){
        digitalWrite(ledPins[2], LED_ON); //the visual mode indicator
      }
      else if(buttonPushCounter % 5 == 3){
        digitalWrite(ledPins[3], LED_ON); //the visual mode indicator
      }
      else if(buttonPushCounter % 5 == 4){
        digitalWrite(ledPins[4], LED_ON); //the visual mode indicator
      }
      delay(2000); //delay 2000 milliseconds
      for(int led = 0; led < ledCount; led ++){
        digitalWrite(ledPins[led], LED_OFF); //make sure everything's off again
      }
    }
  
    //actually execute the modes
    if(buttonPushCounter % 5 == 0){
      Default();   
    }
    else if(buttonPushCounter % 5 == 1){
      delay(2000);
      DisplayTemp();
    }
    else if(buttonPushCounter % 5 == 2){
      delay(2000);
      DisplayHumidity();
    }
    else if(buttonPushCounter % 5 == 3){
      delay(50); //only a 50 ms delay between photoresistor readings
      LightMode();
    }
    else if(buttonPushCounter % 5 == 4){
      delay(500); //only a 500 ms delay between songs
      SoundLightMode();
    }
    else{
      Default();
    }
  }
  
  else{ //no extended projects, just loop through modes 1,2,3
    buttonState = digitalRead(buttonPin); //check button state
    if(buttonState == HIGH && millis() - time > debounce){ //test if button being pushed
      //indicates off -> on
      buttonPushCounter++; //increment count of push number
      time = millis(); //keep track of time
      Quick(); //visual mode change indicator
      if(buttonPushCounter % 3 == 0){
        digitalWrite(ledPins[0], LED_ON); //visual mode indicator
      }
      else if(buttonPushCounter % 3 == 1){
        digitalWrite(ledPins[1], LED_ON); //visual mode indicator
      }
      else if(buttonPushCounter % 3 == 2){
        digitalWrite(ledPins[2], LED_ON); //visual mode indicator
      }
      delay(2000); //delay 2000 milliseconds
      for(int led = 0; led < ledCount; led ++){
        digitalWrite(ledPins[led], LED_OFF); //make sure everything's turned off
      }
    }
  
    //actually execute the modes
    if(buttonPushCounter % 3 == 0){
      Default();   
    }
    else if(buttonPushCounter % 3 == 1){
      delay(2000);
      DisplayTemp();
    }
    else if(buttonPushCounter % 3 == 2){
      delay(2000);
      DisplayHumidity();
    }
    else{
      Default();
    }
  }
}
