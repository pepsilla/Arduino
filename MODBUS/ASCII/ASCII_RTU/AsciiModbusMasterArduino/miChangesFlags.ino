void printRegFlags(unsigned char pointerIni, unsigned char nofRegs){
  unsigned char pointer;
  unsigned char pointerEnd = pointerIni+nofRegs;
  unsigned char registerFlag;
  unsigned char registerBit;
  unsigned int value;
  Serial.print("[");
  Serial.print(pointerIni);
  Serial.print(":");
  Serial.print(nofRegs);
  Serial.print("],");
  for (pointer = pointerIni; pointer<pointerEnd;pointer++){
    registerFlag = pointer / 16;
    registerBit = pointer % 16;
    value = 1;
    value <<= (15-registerBit);
    //Serial.print(value);
    //Serial.print(",");
    value = change_flags[registerFlag] & value;
    //Serial.println(value);
    if(value>0){
     Serial.print("[");
     Serial.print(pointer);
     Serial.print(":");
     Serial.print(regs[pointer],16);
     Serial.print("],");
    }
    change_flags[registerFlag] &= ~value;
  }
  Serial.println("END");
}
