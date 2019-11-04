#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 rtc;

long EndTime, StartTime;

class Indicator{
  public: double value[3] = {0,0,0};
  };
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Работает лучше



class EventEnc{
  public: 
      int numRevol = 0;
      byte numClick = 0;
      byte numShortClick = 1;
      bool shortPress = false;
      bool longClick = false;
      bool longPress = false;
      bool turnLeft = false;
      bool turnRight = false;

  public: void ResetValues(){
    shortPress = false;
    longPress = false;
    turnLeft = false;
    turnRight = false;
    }
  public: void clickLong(){
    longClick = !longClick;
  }  
  };
class Encoder123{
    long timeEnd, timeStart, timeButStart, timeButEnd = 0;
    const int push = 5;
    byte whither;
    public: byte encoder(byte pinA, byte pinB, byte pinBut, EventEnc &event) {
      timeEnd = millis();
      if (timeEnd - timeStart >= push) {
        timeStart = timeEnd;
        byte encoderA = digitalRead(pinA);
        byte encoderB = digitalRead(pinB);
        byte button = digitalRead(pinBut);
        if (encoderA && !event.turnLeft && !event.turnRight) {
          event.turnLeft = true;
          return event.turnLeft;
        }
        if (encoderB && !event.turnLeft && !event.turnRight) {
          event.turnRight = true;
          return event.turnRight;
        }
        if ((!encoderA && !encoderB) && (event.turnLeft || event.turnRight)) {
          event.ResetValues();
        }
        
        if (button ) {
          timeButStart = millis();
          if (!event.shortPress) {
            timeButEnd = timeButStart;
            event.shortPress = true;
          }
          if (timeButStart - timeButEnd >= 500 && !event.longPress ) {
            timeButEnd = timeButStart;
            event.longPress = true;
            event.clickLong();
            return event.longPress;
          }
        }
        if (!button && event.shortPress) {
          if (!event.longPress) {
            event.numShortClick++;
          }
          event.ResetValues();
          return true;
        }
      }
      return false;
    }
};

EventEnc eve;
Encoder123 enc;
Indicator disp;
byte pos = 1;
void onDisplay(double volt, double amper, double timer, int h, int m, int s){
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.setCursor(80,16);
    u8g2.print(volt);
    u8g2.setCursor(80,32);
    u8g2.print(amper);
    u8g2.setCursor(80,48);
    u8g2.print(timer);
    u8g2.setCursor(25,64);
    u8g2.print(h);
    u8g2.setCursor(50,64);
    u8g2.print(m);
    u8g2.setCursor(75,64);
    u8g2.print(s);
  }
void pointer(byte &pos){
  if(pos > 2){
    pos = 0;
    }
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.setCursor(0,16*(pos+1));
    u8g2.print(">");
  }
void firstScreen(){
    u8g2.setFont(u8g2_font_ncenB12_tr);
    u8g2.setCursor(12,16);
    u8g2.print("Vol:");
    u8g2.setCursor(12,32);
    u8g2.print("Amp:");
    u8g2.setCursor(12,48);
    u8g2.print("Time:");
  }
void setup() {
  Serial.begin(57600);
  pinMode(2, INPUT_PULLUP); // определяем вывод как вход
  pinMode(4, INPUT_PULLUP); // определяем вывод как вход
  u8g2.begin();
  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
   if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
}
void loop() {
  Serial.available();
  DateTime now = rtc.now();

  if(enc.encoder(2, 4, 3, eve)){
    if(eve.turnRight){
      disp.value[eve.numShortClick]++;
      }
    if(eve.turnLeft){
      disp.value[eve.numShortClick]--;
      }
    }
  EndTime = millis();
  if(EndTime - StartTime >= 150){
    StartTime = EndTime;
    u8g2.clearBuffer();
    firstScreen();
    int h = now.hour();
    int m = now.minute();
    int s = now.second();
    if(eve.longClick){
      pointer(eve.numShortClick);
      onDisplay(disp.value[0], disp.value[1], disp.value[2], h,m,s);
      }else{
        onDisplay(111, 222, 333, h,m,s);
        }
    
    u8g2.sendBuffer(); 
    }
}
