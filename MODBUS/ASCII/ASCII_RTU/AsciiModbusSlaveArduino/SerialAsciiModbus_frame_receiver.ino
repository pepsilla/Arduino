boolean frameInit=false;
boolean frameEnd=false;
unsigned char receivedByte;
boolean isSecondbyte=false;

void serialEvent2()
{
  while(Serial2.available()){
  unsigned char receivedChar=Serial2.read();
  //Serial.write(receivedChar);
  //Serial.write('=');
  if(frameInit)
  {
    if(receivedChar>47 && receivedChar<58)
    {
      receivedChar -= 48;//0...9 -> 0...9
      //loByte Ascii
      if (isSecondbyte){
        receivedByte |= receivedChar;
        isSecondbyte=false; //Set for next hiByte Ascii
        frameInit=putchar_in_frame(receivedByte);//if overflow wait for next init frame char.
        //Serial.print(receivedByte,16);
        //Serial.write(',');
      }
      //hiByte Ascii
      else{
        receivedByte = receivedChar << 4;
        isSecondbyte = true;
      }
    }
    else if(receivedChar>64 && receivedChar<71){
      receivedChar -= 55;//A...F -> 10...15
      //loByte Ascii
      if (isSecondbyte){
        receivedByte |= receivedChar;
        isSecondbyte=false; //Set for next hiByte Ascii
        frameInit=putchar_in_frame(receivedByte);//if overflow wait for next init frame char.
        //Serial.print(receivedByte,16);
        //Serial.write(',');
      }
      //hiByte Ascii
      else{
        receivedByte = receivedChar << 4;
        isSecondbyte = true;
      }
    }
    else if(receivedChar==':')init_frame();
    else if(receivedChar=='\r' && isSecondbyte==false){
      isSecondbyte = true;
      receivedByte = 0;
    }
    else if(receivedChar=='\n' && isSecondbyte){
      isSecondbyte = false;
      frameEnd=true;
      
      receivedByte = 0;
      end_frame();
      frameInit=false;
    }
  }
  else if(receivedChar==':')
  {
    init_frame();
    frameInit=true;
    frameEnd=false; 
  }
  //Serial.write(',');
  }
  Serial.flush();
}
