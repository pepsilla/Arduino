#include <AsciiModbusMaster.h>
#include <SerialCommand.h>

/*
   The example will use packet1 to read a register from address 0 (the adc ch0 value)
   from the arduino slave (id=1). It will then use this value to adjust the brightness
   of an led on pin 9 using PWM.
   It will then use packet2 to write a register (its own adc ch0 value) to address 1 
   on the arduino slave (id=1) adjusting the brightness of an led on pin 9 using PWM.
*/

/*
@pepsilla:
	ASCII Modbus protocol.
	From SimpleModbusMaster RTU Modbus protocol.
	Frame detect in SerialEvent2.
	Serial2. Hardware Serial.
	38400, SERIAL_8N2
@ver:
	20151002:Pre initial test, run analisys.
*/  

//////////////////// Port information ///////////////////
#define baud 38400
#define t_timeout 500
#define t_polling 200 // the scan rate
#define t_retry_count 240

#define timeout 500
#define polling 0
#define retry_count 2400
//boolean _debugMode = false;

// used to toggle the receive/transmit pin on the driver
#define TxEnablePin 2 

#define LED 13

// The total amount of available memory on the master to store data
#define TOTAL_NO_OF_REGISTERS 240
#define TOTAL_NO_OF_REGISTERS_CHANGE_FLAGS (TOTAL_NO_OF_REGISTERS/16)+1

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  PACKET1,
  PACKET2,
  PACKET3,
  //PACKET4,
  TOTAL_NO_OF_PACKETS // leave this last entry
};



// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS+1];

// Masters register array
unsigned int regs[TOTAL_NO_OF_REGISTERS];
//
unsigned int change_flags[TOTAL_NO_OF_REGISTERS_CHANGE_FLAGS];

SerialCommand _myMonitor;

boolean _autoChangeFlags = false; //Turn on/off change notifications

void setup()
{
  // Initialize each packet
  modbus_construct(&packets[PACKET1], 10, READ_HOLDING_REGISTERS, 0,27 , 0);
  modbus_construct(&packets[PACKET2], 11, READ_HOLDING_REGISTERS, 0, 9, 28);
  modbus_construct(&packets[PACKET3], 52, READ_HOLDING_REGISTERS, 0, 54, 38);
  //modbus_construct(&packets[PACKET4], 11, READ_HOLDING_REGISTERS, 0, 9, 10);
  // Initialize the Modbus Finite State Machine
  modbus_configure(&Serial2, baud, SERIAL_8N2, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS, regs, change_flags);
  
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  //Set communication command/response for USB Serial port
  _myMonitor.addCommand("R",getREGISTERS); // Print modbus registers
  _myMonitor.addCommand("G",getStatus); // Print debug registers
  _myMonitor.addCommand("to",setTimeOut); // Set/Get Time out of slave response
  _myMonitor.addCommand("po",setPolling); // Set/Get Time polling of Modbus serial
  _myMonitor.addCommand("re",setRetries); // Set/Get number of retry for timeouts
  _myMonitor.addCommand("degubSerial",setDebug); // Set/Get debug info to Serial port
  _myMonitor.addCommand("cf" ,changeFlags); //Set/Get serial.print changes notifications.
  
  //_myMonitor.addCommand("date",setFecha); // Set date from string ("YYYY/MM/DD") and broadcast to ModBus
  //_myMonitor.addCommand("time",setTiempo); // Set time form string ("HH:MM:SS") and broadcast to ModBus
  
  _myMonitor.addDefaultHandler(miMonitor);//Default handler ex:for decive information
  
}

void loop()
{
  boolean my;
  
  my=modbus_update();
  _myMonitor.readSerial();
  
  //Evaluate changes when packet decoded
  if(my && _autoChangeFlags){
    unsigned char counterPacket;
    for(counterPacket=0; counterPacket<TOTAL_NO_OF_PACKETS;counterPacket++){
      if(packets[counterPacket].change_flags)
      {
        /*
        modbus_construct(&packets[PACKET1], 10, READ_HOLDING_REGISTERS, 0,27 , 0);
        modbus_construct(&packets[PACKET2], 11, READ_HOLDING_REGISTERS, 0, 9, 28);
        modbus_construct(&packets[PACKET3], 52, READ_HOLDING_REGISTERS, 0, 54, 38);
        */
        switch(counterPacket){
          case 0: printRegFlags(0,27);
                  break;
          case 1: printRegFlags(28,9);
                  break;
          case 2: printRegFlags(38,54);
                  break;
          default: Serial.println("[Check counterPacket switch],WARNING");
        }
        packets[counterPacket].change_flags = false;
      }
    }
  }


  
}


