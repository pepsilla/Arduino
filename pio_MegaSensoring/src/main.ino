#include "Arduino.h"
#include "DHT.h"

#define DHTPIN 8
#define DHTTYPE DHT22

byte anIN[] = {A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15};
String labelChanel[] = {"MQ-2","MQ-3","MQ-4","MQ-5","MQ-6","MQ-7","MQ-8","MQ-9","MQ-135","NONE","NONE","NONE","NONE","NONE","NONE","ACS712"};
byte numberOfAi = sizeof(anIN);

unsigned int analogValues[sizeof(anIN)];
unsigned long numLoops = 0;
unsigned long toOneSecond = 0;
unsigned int framerPerSecond = 0;

unsigned int counterFourSeconds = 0;

DHT dht(DHTPIN, DHTTYPE);

void setup(){
  Serial.begin(115200);
  pinMode(DHTPIN,INPUT_PULLUP);
  dht.begin();
  byte counter = 0;
  while (counter < numberOfAi){
    analogValues[counter] = 0;
    counter ++;
  }
  delay(5000);
  //Serial.println("IN:0");
}

String hex(unsigned int value){
  String returnValue = "";
  byte toHex;
  toHex = (0xF000 & value)>>12;
  returnValue += String(toHex,16);
  toHex = (value & 0xF00)>>8;
  returnValue += String(toHex,16);
  toHex = (value & 0xF0)>>4;
  returnValue += String(toHex,16);
  toHex = (value & 0xf);
  returnValue += String(toHex,16);;
  return returnValue;
  toOneSecond = millis();
}

void loop(){

  if(millis()-toOneSecond >= 1000){
    framerPerSecond = numLoops;
    numLoops = 0;
    toOneSecond = millis();
    counterFourSeconds ++;
  }

  if(counterFourSeconds == 4){
    counterFourSeconds = 0;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("TEMP:888,HUMI:888");
      numLoops ++;
      delay(1000);
      return;
    }
    float hic = dht.computeHeatIndex(t, h, false);
    Serial.print("HUMI:");
    Serial.print(h);
    Serial.print(",TEMP:");
    Serial.print(t);
    Serial.print(",INDX:");
    Serial.println(hic);
  }
  else{
    byte counter = 0;
    unsigned int analogValue = 0;
    //unsigned long initTime = millis();
    //Serial.print(numLoops, 16);
    //Serial.print(":[");
    while(counter<numberOfAi){
      analogValue = analogRead(anIN[counter]);
      Serial.print(labelChanel[counter]);
      //if(counter < 10) Serial.print("0");
      //Serial.print(counter);
      //Serial.print("]=");
      Serial.print(":");
      Serial.print(analogValue);
      //Serial.print(analogValue,16);
      if(analogValue != analogValues[counter]){
        analogValues[counter] = analogValue;
      }
      counter ++;
      if(counter<numberOfAi)Serial.print(",");
    }
    Serial.print(",IN:");
    //Serial.print(millis() - initTime);
    Serial.print(framerPerSecond);
    Serial.println("");
  }
  numLoops ++;
}
