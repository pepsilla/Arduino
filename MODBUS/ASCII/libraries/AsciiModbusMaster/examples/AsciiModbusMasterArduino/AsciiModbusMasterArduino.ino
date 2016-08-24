#include <AsciiModbusMaster.h>

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
#define timeout 1000
#define polling 125 // the scan rate
#define retry_count 24

// used to toggle the receive/transmit pin on the driver
#define TxEnablePin 2 

#define LED 13

// The total amount of available memory on the master to store data
#define TOTAL_NO_OF_REGISTERS 120

// This is the easiest way to create new packets
// Add as many as you want. TOTAL_NO_OF_PACKETS
// is automatically updated.
enum
{
  PACKET1,
  PACKET2,
  PACKET3,
  TOTAL_NO_OF_PACKETS // leave this last entry
};

// Create an array of Packets to be configured
Packet packets[TOTAL_NO_OF_PACKETS];

// Masters register array
unsigned int regs[TOTAL_NO_OF_REGISTERS];

void setup()
{
  // Initialize each packet
  modbus_construct(&packets[PACKET1], 10, READ_HOLDING_REGISTERS, 0,9 , 0);
  modbus_construct(&packets[PACKET2], 11, READ_HOLDING_REGISTERS, 0, 9, 10);
  modbus_construct(&packets[PACKET2], 52, READ_HOLDING_REGISTERS, 0, 54, 19);
  // Initialize the Modbus Finite State Machine
  modbus_configure(&Serial2, baud, SERIAL_8N2, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS, regs);
  
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
}

void loop()
{
  modbus_update();
  
  regs[110] = analogRead(0); // update data to be written to arduino slave
  regs[1] = analogRead(1); // update data to be written to arduino slave
  regs[2] = analogRead(2); // update data to be written to arduino slave
  regs[3] = analogRead(3); // update data to be written to arduino slave
  regs[4] = analogRead(4); // update data to be written to arduino slave
  regs[5] = analogRead(5); // update data to be written to arduino slave
  regs[6] = analogRead(6); // update data to be written to arduino slave
  regs[7] = analogRead(7); // update data to be written to arduino slave
  regs[8] = analogRead(8); // update data to be written to arduino slave
  regs[9] = analogRead(9); // update data to be written to arduino slave
  regs[10] = analogRead(10); // update data to be written to arduino slave
  regs[11] = analogRead(11); // update data to be written to arduino slave
  regs[12] = analogRead(12); // update data to be written to arduino slave
  regs[13] = analogRead(13); // update data to be written to arduino slave
  regs[14] = analogRead(14); // update data to be written to arduino slave
  regs[15] = analogRead(15); // update data to be written to arduino slave
  
  analogWrite(LED, regs[0]>>2); // constrain adc value from the arduino slave to 255
}
