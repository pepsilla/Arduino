/*
 * MIDIUSB_test.ino
 *
 * Created: 4/6/2015 10:47:08 AM
 * Author: gurbrinder grewal
 * Modified by Arduino LLC (2015)
 */ 

#include "MIDIUSB.h"

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

const byte notes [64] = {
  39,47,55,63,71,79,87,7,
  38,46,54,62,70,78,86,7,
  37,45,53,61,69,77,85,7,
  36,44,52,60,68,76,84,7,
  00,43,51,59,67,75,83,7,
  00,42,50,58,66,74,82,7,
  00,41,49,57,65,73,81,7,
  00,40,48,56,64,72,80,7,
};

byte key1[8],key2[8];

byte getBite(byte valor, byte pos){
  byte exponente = pow(2,pos);
  byte resultado = valor & exponente;
  resultado >>= pos;
  return resultado;
}
void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  //Serial.println("noteON");
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  //Serial.println("noteOFF");
}

void scanKeyBoard(){
  byte row,column;
  for (row=22;row<30;row++){
    digitalWrite(row,HIGH);
    //Serial.print(row);
    //Serial.print("\t");
    byte key1Pointer = 0;
    for(column=30;column<44;column+=2){
      byte valor = digitalRead(column);
      byte estado = getBite(key1[row-22],key1Pointer);
      //Serial.print(estado);
      //Serial.print(",");
      //Serial.print(valor);
      //Serial.print(" ");
      if(estado != valor) {
        byte exponente = pow(2,key1Pointer);
        byte nota = (row-22)*8+key1Pointer;
        //Cambios detectados
        if(valor){
          //Serial.println();
          //Serial.println(key1[row-22]);
          noteOn(0,notes[nota], 64);   // Channel 0, middle C, normal velocity
          key1[row-22]|= exponente;
          Serial.println("noteON: ");
          Serial.print(row-22);
          Serial.print(",");
          Serial.println(key1Pointer);
        }
        else {
          noteOff(0, notes[nota], 64);  // Channel 0, middle C, normal velocity
          exponente = ~exponente;
          key1[row-22] &= exponente;
          Serial.println("noteOFF: ");
          Serial.print(row-22);
          Serial.print(",");
          Serial.println(key1Pointer);
        }
        MidiUSB.flush();
        key1Pointer++;
      }
      key1Pointer++;
    }
    //Serial.print(key1[row-22],2);
    //Serial.println();
    digitalWrite(row,LOW);
  }
}

void setup() {
  Serial.begin(115200);
  byte counter;
  for (counter = 22;counter<30;counter++)pinMode(counter,OUTPUT);
  for (counter=30;counter<44;counter++)pinMode(counter,INPUT);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void loop() {
     scanKeyBoard();
 }
