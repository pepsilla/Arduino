#include <SimpleModbusSlave.h>
/*
@pepsilla:
	ASCII Modbus protocol.
	From SimpleModbusSlave library (RTU Modbus protocol).
	Frame detect in SerialEvent2.
	Serial2. Hardware Serial.
	38400, SERIAL_8N2
@ver:
	20151002:Pre initial test, run analisys.
*/ 

/* 
   SimpleModbusSlaveV10 supports function 3, 6 & 16.
   
   This example code will receive the adc ch0 value from the arduino master. 
   It will then use this value to adjust the brightness of the led on pin 9.
   The value received from the master will be stored in address 1 in its own
   address space namely holdingRegs[].
   
   In addition to this the slaves own adc ch0 value will be stored in 
   address 0 in its own address space holdingRegs[] for the master to
   be read. The master will use this value to alter the brightness of its
   own led connected to pin 9.
   
   The modbus_update() method updates the holdingRegs register array and checks
   communication.

   Note:  
   The Arduino serial ring buffer is 64 bytes or 32 registers.
   Most of the time you will connect the arduino to a master via serial
   using a MAX485 or similar.
 
   In a function 3 request the master will attempt to read from your
   slave and since 5 bytes is already used for ID, FUNCTION, NO OF BYTES
   and two BYTES CRC the master can only request 58 bytes or 29 registers.
 
   In a function 16 request the master will attempt to write to your 
   slave and since a 9 bytes is already used for ID, FUNCTION, ADDRESS, 
   NO OF REGISTERS, NO OF BYTES and two BYTES CRC the master can only write
   54 bytes or 27 registers.
 
   Using a USB to Serial converter the maximum bytes you can send is 
   limited to its internal buffer which differs between manufactures. 
*/

#define  LED 13  
#define baud 38400

// Using the enum instruction allows for an easy method for adding and 
// removing registers. Doing it this way saves you #defining the size 
// of your slaves register array each time you want to add more registers
// and at a glimpse informs you of your slaves register layout.

//////////////// registers of your slave ///////////////////

#define HOLDING_REGS_SIZE 120// leave this one
  // total number of registers for function 3 and 16 share the same register array
  // i.e. the same address space


unsigned int holdingRegs[HOLDING_REGS_SIZE]; // function 3 and 16 register array
////////////////////////////////////////////////////////////

void setup()
{
  /* parameters(HardwareSerial* SerialPort,
                long baudrate, 
		unsigned char byteFormat,
                unsigned char ID, 
                unsigned char transmit enable pin, 
                unsigned int holding registers size,
                unsigned int* holding register array)
  */
  
  /* Valid modbus byte formats are:
     SERIAL_8N2: 1 start bit, 8 data bits, 2 stop bits
     SERIAL_8E1: 1 start bit, 8 data bits, 1 Even parity bit, 1 stop bit
     SERIAL_8O1: 1 start bit, 8 data bits, 1 Odd parity bit, 1 stop bit
     
     You can obviously use SERIAL_8N1 but this does not adhere to the
     Modbus specifications. That said, I have tested the SERIAL_8N1 option 
     on various commercial masters and slaves that were suppose to adhere
     to this specification and was always able to communicate... Go figure.
     
     These byte formats are already defined in the Arduino global name space. 
  */
	
  modbus_configure(&Serial2, baud, SERIAL_8N2, 1, 2, HOLDING_REGS_SIZE, holdingRegs);

  // modbus_update_comms(baud, byteFormat, id) is not needed but allows for easy update of the
  // port variables and slave id dynamically in any function.
  modbus_update_comms(baud, SERIAL_8N2, 1);
  
  pinMode(LED, OUTPUT);
  Serial.begin(115200);// For debug propouses;
}

void loop()
{
  // modbus_update() is the only method used in loop(). It returns the total error
  // count since the slave started. You don't have to use it but it's useful
  // for fault finding by the modbus master.
  
  modbus_update();
  
  holdingRegs[0] = analogRead(A0); // update data to be read by the master to adjust the PWM
  
  analogWrite(LED, holdingRegs[110]>>2); // constrain adc value from the arduino master to 255
  holdingRegs[1] = analogRead(A0); // update data to be read by the master to adjust the PWM
  holdingRegs[2] = analogRead(A1); // update data to be read by the master to adjust the PWM
  holdingRegs[3] = analogRead(A2); // update data to be read by the master to adjust the PWM
  holdingRegs[4] = analogRead(A3); // update data to be read by the master to adjust the PWM
  holdingRegs[5] = analogRead(A4); // update data to be read by the master to adjust the PWM
  holdingRegs[6] = analogRead(A5); // update data to be read by the master to adjust the PWM
  holdingRegs[7] = analogRead(A6); // update data to be read by the master to adjust the PWM
  holdingRegs[8] = analogRead(A7); // update data to be read by the master to adjust the PWM
  holdingRegs[9] = analogRead(A8); // update data to be read by the master to adjust the PWM
  holdingRegs[10] = analogRead(A9); // update data to be read by the master to adjust the PWM
  holdingRegs[11] = analogRead(A10); // update data to be read by the master to adjust the PWM
  holdingRegs[12] = analogRead(A11); // update data to be read by the master to adjust the PWM
  holdingRegs[13] = analogRead(A12); // update data to be read by the master to adjust the PWM
  holdingRegs[14] = analogRead(A13); // update data to be read by the master to adjust the PWM
  holdingRegs[15] = analogRead(A14); // update data to be read by the master to adjust the PWM
  holdingRegs[16] = analogRead(A15); // update data to be read by the master to adjust the PWM
  /* Note:
     The use of the enum instruction is not needed. You could set a maximum allowable
     size for holdinRegs[] by defining HOLDING_REGS_SIZE using a constant and then access 
     holdingRegs[] by "Index" addressing. 
     I.e.
     holdingRegs[0] = analogRead(A0);
     analogWrite(LED, holdingRegs[1]/4);
  */
  
}

