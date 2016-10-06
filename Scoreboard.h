#include <Arduino.h>
#include <TM1637Display.h>

// Display Pins
#define CLK 4
#define DIO 5

#define minDisplaySettingDelayTime 100

// display power status
bool displayOn = false;

// last time setting the display
unsigned long lastDisplaySettingTime = 0; 
unsigned long lastDisplayToggleTime = 0; 

//const uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };

TM1637Display display(CLK, DIO);
 
void clearDisplay(){
 Serial.println("clear");
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
