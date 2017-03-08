import controlP5.*;
import processing.serial.*;
//import cc.arduino.*;
import arduinoscope.*;

//Arduino arduino;
ControlP5 cp5;
Oscilloscope[] scopes = new Oscilloscope[16];
float multiplier;

String labelChanel[] = new String[17];
int valueChanel[] = new int[17];
String otherValues;
Serial miPuerto;
int Baud = 115200;

void setup(){
  size(1600, 1024);
  ControlP5 cp5 = new ControlP5(this);
  frame.setTitle("Arduinoscope");
  
  miPuerto = new Serial(this, "COM11",Baud);//Serial.list()[Puerto], Baud);  // Abre el puerto que se esté utilizando
  miPuerto.bufferUntil('\n');  // No genera un serialEvent() a menos que se obtenga un caracter de nueva línea ('\n')
  
  // COM dropdown
  DropdownList com = cp5.addDropdownList("com")
    .setPosition(210, 20)
    .setSize(200,200);
  /*String[] arduinos = arduino.list();
  for (int i=0; i<arduinos.length; i++) {
    com.addItem(arduinos[i], i);
  }
  */
  int[] dim = { 800-130, height/(scopes.length/2)};
  
  for (int i=0;i<scopes.length/2;i++){
    int[] posv = new int[2];
    posv[0]=0;
    posv[1]=dim[1]*i;
  print (posv[0],posv[1],i);
  print("\n");
    // random color, that will look nice and be visible
    scopes[i] = new Oscilloscope(this, posv, dim);
    scopes[i].setLine_color(color((int)random(255), (int)random(127)+127, 255));
    
    cp5.addButton("pause" + i)
      .setLabel("pause")
      .setValue(i)
      .setPosition(dim[0]+10,posv[1] + 85)
      .updateSize();
     
    scopes[i].setPause(false);
  }
  
  
  for (int i=scopes.length/2;i<scopes.length;i++){
    int[] posv = new int[2];
    posv[0]=800;
    posv[1]=dim[1]*(i-(scopes.length/2));
  print (posv[0],posv[1],i);
  print("\n");
    // random color, that will look nice and be visible
    scopes[i] = new Oscilloscope(this, posv, dim);
    scopes[i].setLine_color(color((int)random(255), (int)random(127)+127, 255));
    
    cp5.addButton("pause" + i)
      .setLabel("pause")
      .setValue(i)
      .setPosition(1470+10,posv[1] + 85)
      .updateSize();
     
    scopes[i].setPause(false);
  }
  
  
  // multiplier comes from 1st scope
  multiplier = scopes[0].getMultiplier()/scopes[0].getResolution();
  
}

void draw(){
  background(0);
  text("eScope "+labelChanel[16]+":"+valueChanel[16]+" m/s.", 20, 20);
  frame.setTitle("SerialWHEATER:"+otherValues);
  
  int val;
  int[] dim;
  int[] pos;
  stroke(255);
  line(800,0,800,height);
  
  for (int i=0;i<scopes.length/2;i++){
    dim = scopes[i].getDim();
    pos = scopes[i].getPos();
    //print(pos[0],pos[1],dim[0],dim[1],i,"\n");
    scopes[i].drawBounds();
    stroke(255);
    line(0, pos[1], width, pos[1]);
    
    scopes[i].draw();
    
      text("analog " + labelChanel[i], dim[0]+10, pos[1] + 30);
      text("val: " + (valueChanel[i]*multiplier) + "V", dim[0]+10, pos[1] + 45);
      text("min: " + (scopes[i].getMinval()*multiplier) + "V", dim[0]+10, pos[1] + 60);
      text("max: " + (scopes[i].getMaxval()*multiplier) + "V", dim[0]+10, pos[1] + 75);
    
  }
  
  for (int i=scopes.length/2;i<scopes.length;i++){
    dim = scopes[i].getDim();
    pos = scopes[i].getPos();
    
    scopes[i].drawBounds();
    stroke(255);
    
    scopes[i].draw();
    if(i==15){
      text("analog " + labelChanel[i], dim[0]+810, pos[1] + 30);
       text("raw: " + (valueChanel[i]), dim[0]+510, pos[1] + 45);
       text("raw: " + (valueChanel[i]*0.004885)+"V", dim[0]+650, pos[1] + 45);
      text("val: " + (((valueChanel[i] * 4.882)-2500)/0.185) + "mA", dim[0]+810, pos[1] + 45);
      text("min: " + (((scopes[i].getMinval() * 4.882)-2500)/0.185) + "mA", dim[0]+810, pos[1] + 60);
      text("max: " + (((scopes[i].getMaxval() * 4.882)-2500)/0.185) + "mA", dim[0]+810, pos[1] + 75);
    }
    else{
      text("analog " + labelChanel[i], dim[0]+810, pos[1] + 30);
      text("val: " + (valueChanel[i]*multiplier) + "V", dim[0]+810, pos[1] + 45);
      text("min: " + (scopes[i].getMinval()*multiplier) + "V", dim[0]+810, pos[1] + 60);
      text("max: " + (scopes[i].getMaxval()*multiplier) + "V", dim[0]+810, pos[1] + 75);
    }
    
  }
  
}


void controlEvent(ControlEvent theEvent) {
  int val = int(theEvent.getValue());
  
  if (theEvent.getName() == "com"){
    miPuerto.stop();
  }else{    
    scopes[val].setPause(!scopes[val].isPause());
  }
}

void serialEvent (Serial miPuerto) { // Lee los datos capturados por el puerto del Arduino
     
     String StringEntrante = miPuerto.readStringUntil('\n');  // Obtiene la cadena ASCII
 
     if (StringEntrante != null) {
         String StringValores[] = split(StringEntrante,",");  // divide valores
         int parValues = StringValores.length;
         //print(StringEntrante);
         if(parValues==17){
           for (int counter=0;counter<parValues;counter++){
             String parValor[]=StringValores[counter].split(":");//recoge nombre/valor
             if(parValor.length == 2){
               parValor[1] = trim(parValor[1]);
               int valueData = int(parValor[1]);
               valueChanel[counter]=valueData;
               if(counter < parValues-1)scopes[counter].addData(valueData);
               labelChanel[counter] = parValor[0];
             }
           }
         }
         else if(parValues==2 || parValues==3)otherValues = trim(StringEntrante); 
  }
}
