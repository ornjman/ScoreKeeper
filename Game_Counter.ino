#include <SPI.h>
#include "RF24.h"
//#include "Scoreboard.h"

#include <Arduino.h>
#include <TM1637Display.h>

// Display Pins
#define CLK 4
#define DIO 5
#define minDisplaySettingDelayTime 100

TM1637Display display(CLK, DIO);

// display power status
bool displayOn = false;

bool newScoreSet = false;

// last time setting the display
unsigned long lastDisplaySettingTime = 0; 
unsigned long lastDisplayToggleTime = 0; 

// Naming Conventions:
// TX/tX = Team X


// Team 1 Score Pins
#define T1UPPIN 2
#define T1DOWNPIN 1

// Team 2 Score Pins
#define T2UPPIN 3
#define T2DOWNPIN 1

// define clear pin
#define CLEARPIN 3

#define gameCounterDisplayNumber 1

RF24 radio(7,8);
byte addresses[][6] = {"team1","team2"};

// set maximum score for which to wrap around
int maximumScore = 21;

volatile int team1Score = 0;
volatile int team2Score = 0;

uint8_t data[] = { 0x4f, 0x3f, 0x6f, 0x8f };
const uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

void setup() {
  
  Serial.begin(115200);
  
  // setup score buttons
  pinMode(T1UPPIN, INPUT_PULLUP);
  pinMode(CLEARPIN, INPUT_PULLUP);
  
  // attach interrupt for point buttons
  attachInterrupt(digitalPinToInterrupt(T1UPPIN), incrementTeam1Score, FALLING);
  attachInterrupt(digitalPinToInterrupt(CLEARPIN), reset, FALLING);

  // turn on display
  toggleDisplayPower();

  radio.begin();

  radio.setPALevel(RF24_PA_LOW);
  
   // Open a writing and reading pipe on each radio, with opposite addresses
  if(gameCounterDisplayNumber == 1){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else if(gameCounterDisplayNumber == 2){
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  radio.startListening();
}

void loop() {

  // check for any winners
  if(team1Score == maximumScore || team2Score == maximumScore){
    // toggle display power every .5 second
    if((millis() - lastDisplayToggleTime) > 500){
      toggleDisplayPower();
    }
  }else{
    if(!displayOn){
      toggleDisplayPower();
    }
  }
  // need to add code to check for values changed
  if((millis() - lastDisplaySettingTime) > minDisplaySettingDelayTime){
    if(team1Score >= 10){
      data[0] = display.encodeDigit(team1Score/10);
    }else{
      data[0] = 0;
    }
    data[1] = display.encodeDigit(team1Score%10);
    if(team2Score >= 10){
      data[2] = display.encodeDigit(team2Score/10);
    }else{
      data[2] = 0;
    }
    data[3] = display.encodeDigit(team2Score%10);
    
    setDisplayData(data);
    //display.setSegments(data);

    lastDisplaySettingTime = millis();
  }
  
  if(newScoreSet){
    sendNewScore();
  }
  receiveNewScore();
}
void sendNewScore(){
  radio.stopListening();
  if (!radio.write( &team1Score, sizeof(unsigned long) )){
   Serial.println(F("failed"));
  }else{
    newScoreSet = false;
  }
  radio.startListening();
}
void receiveNewScore(){
  
  if(radio.available()){
    unsigned int newScore;
    while (radio.available()) {                                   // While there is data ready
      radio.read( &newScore, sizeof(unsigned long) );             // Get the payload
    }
    team1Score = newScore;
  }
}
void incrementTeam1Score(){
  incrementScore(&team1Score);
}
void incrementTeam2Score(){
  incrementScore(&team2Score);
}
void incrementScore(int *score){
    if(*score + 1 >= maximumScore){
      *score = maximumScore;
    }else{
      *score = (*score + 1) % maximumScore;    
    }
    newScoreSet = true;
}

void reset(){
  
  // reset scores
  team1Score = 0;
  team2Score = 0;
 
  newScoreSet = true;
}
 
void clearDisplay(){
 display.setSegments(blank);
}

void setDisplayData(uint8_t data[]){
  display.setSegments(data);
}
void toggleDisplayPower(){
  if(!displayOn){
    display.setBrightness(0x0F);
    displayOn = true;
  }else{
    display.setBrightness(0x00);
    displayOn = false;
  }
  lastDisplayToggleTime = millis();
}
