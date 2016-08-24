#include "AsciiModbusMaster.h"
#include "HardwareSerial.h"
/*
 *@pepsilla: 
 * 	Ascii Modbus from SimpleModbus
 *  ASCII ["0,1,...9]:[0x30,0x31,...0x39]|[48,49,...57]
 *  ASCII ["A,B,...F"]:[0x41,0x42,...0x46]|[65,66,...70]
 * 
 * 
*/
// SimpleModbusMasterV2rev2

// state machine states
#define IDLE 1
#define WAITING_FOR_REPLY 2
#define WAITING_FOR_TURNAROUND 3

#define BUFFER_SIZE 258

unsigned char state;
unsigned char retry_count;
unsigned char TxEnablePin;

// frame[] is used to receive and transmit packages. 
// The maximum number of bytes in a modbus packet is 256 bytes
// This is limited to the serial buffer of 64 bytes
unsigned char frame[BUFFER_SIZE]; 
unsigned char buffer;
long timeout; // timeout interval
long polling; // turnaround delay interval
unsigned int T1_5; // inter character time out in microseconds
unsigned int frameDelay; // frame time out in microseconds
long delayStart; // init variable for turnaround and timeout delay

boolean is_update;// true if modbus rtu frame is disposed
boolean packet_decoded = false;
unsigned int pointer_buffer;// index to frame buffer serial receiver
unsigned int bytesReceived;// Number of bytes in Ascci frame
int lrc;// Longitudinal Redundancy Check
long t_getPacket;//time in milis when request to slave start
boolean _debugMode = false;

unsigned int total_no_of_packets; 
Packet* packetArray; // packet starting address
Packet* packet; // current packet
unsigned int* register_array; // pointer to masters register array
unsigned int* register_change_flags; // pointer to change flags register array
HardwareSerial* ModbusPort;

// function definitions
void idle();
void constructPacket();
unsigned char construct_F15();
unsigned char construct_F16();
void waiting_for_reply();
void processReply();
void waiting_for_turnaround();
void process_F1_F2();
void process_F3_F4();
void process_F5_F6_F15_F16();
void processError();
void processSuccess();
unsigned int calculateCRC(unsigned char bufferSize);
void sendPacket(unsigned char bufferSize);

unsigned char dec_to_ASCII_HEX(unsigned char decChar);

// Modbus Master State Machine
boolean modbus_update() 
{
	switch (state)
	{
		case IDLE:
		idle();
		break;
		case WAITING_FOR_REPLY:
		waiting_for_reply();
		break;
		case WAITING_FOR_TURNAROUND:
		waiting_for_turnaround();
		break;
	}
	//Serial.print("@");
	if(packet_decoded){
		packet_decoded = false;
		return true;
	}
	return false;
}

void idle()
{
  static unsigned int packet_index;	
	
	unsigned int failed_connections = 0;
	
	unsigned char current_connection;
	
	do
	{		
		if (packet_index == total_no_of_packets) // wrap around to the beginning
			packet_index = 0;
				
		// proceed to the next packet
		packet = &packetArray[packet_index];
		
		// get the current connection status
		current_connection = packet->connection;
		
		if (!current_connection)
		{			
			// If all the connection attributes are false return
			// immediately to the main sketch
			if (++failed_connections == total_no_of_packets)
				return;
		}
		packet_index++;     
    
	// if a packet has no connection get the next one		
	}while (!current_connection); 
		
	constructPacket();
}
  
void constructPacket()
{	 
  packet->requests++;
  frame[0] = packet->id;
  frame[1] = packet->function;
  frame[2] = packet->address >> 8; // address Hi
  frame[3] = packet->address & 0xFF; // address Lo
	
	// For functions 1 & 2 data is the number of points
	// For function 5 data is either ON (0xFF00) or OFF (0x0000)
	// For function 6 data is exactly that, one register's data
  // For functions 3, 4 & 16 data is the number of registers
  // For function 15 data is the number of coils
	
	// The data attribute needs to be intercepted by F5 & F6 because these requests
	// include their data in the data register and not in the masters array
	if (packet->function == FORCE_SINGLE_COIL || packet->function == PRESET_SINGLE_REGISTER) 
		packet->data = register_array[packet->local_start_address]; // get the data
	
	
	frame[4] = packet->data >> 8; // MSB
	frame[5] = packet->data & 0xFF; // LSB
	
	unsigned char frameSize;    
	
  // construct the frame according to the modbus function  
  if (packet->function == PRESET_MULTIPLE_REGISTERS) 
		frameSize = construct_F16();
	else if (packet->function == FORCE_MULTIPLE_COILS)
		frameSize = construct_F15();
	else // else functions 1,2,3,4,5 & 6 is assumed. They all share the exact same request format.
    frameSize = 8; // the request is always 8 bytes in size for the above mentioned functions.
		
	unsigned int crc16 = calculateCRC(frameSize - 2);	
  frame[frameSize - 2] = crc16 >> 8; // split crc into 2 bytes
  frame[frameSize - 1] = crc16 & 0xFF;
  sendPacket(frameSize);

	state = WAITING_FOR_REPLY; // state change
	
	// if broadcast is requested (id == 0) for function 5,6,15 and 16 then override 
  // the previous state and force a success since the slave wont respond
	if (packet->id == 0)
			processSuccess();
}

unsigned char construct_F15()
{
	// function 15 coil information is packed LSB first until the first 16 bits are completed
  // It is received the same way..
  unsigned char no_of_registers = packet->data / 16;
  unsigned char no_of_bytes = no_of_registers * 2; 
	
  // if the number of points dont fit in even 2byte amounts (one register) then use another register and pad 
  if (packet->data % 16 > 0) 
  {
    no_of_registers++;
    no_of_bytes++;
  }
	
  frame[6] = no_of_bytes;
  unsigned char bytes_processed = 0;
  unsigned char index = 7; // user data starts at index 7
	unsigned int temp;
	
  for (unsigned char i = 0; i < no_of_registers; i++)
  {
    temp = register_array[packet->local_start_address + i]; // get the data
    frame[index] = temp & 0xFF; 
    bytes_processed++;
     
    if (bytes_processed < no_of_bytes)
    {
      frame[index + 1] = temp >> 8;
      bytes_processed++;
      index += 2;
    }
  }
	unsigned char frameSize = (9 + no_of_bytes); // first 7 bytes of the array + 2 bytes CRC + noOfBytes 
	return frameSize;
}

unsigned char construct_F16()
{
	unsigned char no_of_bytes = packet->data * 2; 
    
  // first 6 bytes of the array + no_of_bytes + 2 bytes CRC 
  frame[6] = no_of_bytes; // number of bytes
  unsigned char index = 7; // user data starts at index 7
	unsigned char no_of_registers = packet->data;
	unsigned int temp;
		
  for (unsigned char i = 0; i < no_of_registers; i++)
  {
    temp = register_array[packet->local_start_address + i]; // get the data
    frame[index] = temp >> 8;
    index++;
    frame[index] = temp & 0xFF;
    index++;
  }
	unsigned char frameSize = (9 + no_of_bytes); // first 7 bytes of the array + 2 bytes CRC + noOfBytes 
	return frameSize;
}

void waiting_for_turnaround()
{
  if ((millis() - delayStart) > polling)
		state = IDLE;
}


void init_frame()
{
    pointer_buffer = 0;
    is_update = false;
    lrc = 0;
}

boolean putchar_in_frame(unsigned char byte_received)
{
    frame[pointer_buffer] = byte_received;
    pointer_buffer +=1;
    if(pointer_buffer == BUFFER_SIZE)
    {
       pointer_buffer = 0;
       is_update = false;
       return false; 
    }
    return true;
}
void end_frame()
{
    unsigned int contador;
    int invertedlrc = 0;
    
	lrc=0;
    //Sum of all byte except LRC
	for(contador=0;contador<pointer_buffer-2;contador++){
		lrc += frame[contador];
	}
	
    invertedlrc = frame[pointer_buffer-2] << 8;
    invertedlrc |= frame[pointer_buffer-1];
    //lrc=0 if all ok.
    lrc += invertedlrc;
    bytesReceived=pointer_buffer;
    if(lrc == 0)
    {
        pointer_buffer -= 2;
        is_update = true;
    }
    else
    {
       //LRC Checksum error. Wait for next frame
       pointer_buffer = 0;
       is_update = false;
       lrc = 0; 
    }
}

void set_debug(boolean value){
	_debugMode = value;
}

boolean get_debug(){
	return _debugMode;
}

// get the serial data from buffer
void waiting_for_reply()
{
	// true if modbus rtu frame is disposed
	if(is_update == true){
		packet_decoded = true;
		buffer = pointer_buffer;
		long t_now = millis();//For time after request to slave 
		
		if (buffer < 5)
			processError();       
      
		// Modbus over serial line datasheet states that if an unexpected slave 
    // responded the master must do nothing and continue with the time out.
		// This seems silly cause if an incorrect slave responded you would want to
    // have a quick turnaround and poll the right one again. If an unexpected 
    // slave responded it will most likely be a frame error in any event
		else if (frame[0] != packet->id) // check id returned
			processError();
		else
			/*
			Serial.print(bytesReceived);
			Serial.print(" in ");
			Serial.print(t_now-t_getPacket);
			Serial.print(" ms; ");
			Serial.print(bytesReceived/(t_now-t_getPacket));
			Serial.println(" ms/car.");
			* */
			processReply();
	}
	else if ((millis() - delayStart) > timeout) // check timeout
	{
		processError();
		state = IDLE; //state change, override processError() state
	}
}

void processReply()
{
	// combine the crc Low & High bytes
  unsigned int received_crc = ((frame[buffer - 2] << 8) | frame[buffer - 1]); 
  unsigned int calculated_crc = calculateCRC(buffer - 2);
	
	if (calculated_crc == received_crc) // verify checksum
	{
		// To indicate an exception response a slave will 'OR' 
		// the requested function with 0x80 
		if ((frame[1] & 0x80) == 0x80) // extract 0x80
		{
			packet->exception_errors++;
			processError();
		}
		else
		{
			switch (frame[1]) // check function returned
      {
        case READ_COIL_STATUS:
        case READ_INPUT_STATUS:
        process_F1_F2();
        break;
        case READ_INPUT_REGISTERS:
        case READ_HOLDING_REGISTERS:
        process_F3_F4();
        break;
				case FORCE_SINGLE_COIL:
				case PRESET_SINGLE_REGISTER:
        case FORCE_MULTIPLE_COILS:
        case PRESET_MULTIPLE_REGISTERS:
        process_F5_F6_F15_F16();
        break;
        default: // illegal function returned
        processError();
        break;
      }
		}
	} 
	else // checksum failed
	{
		processError();
	}
	is_update = false;
}

void process_F1_F2()
{
	// packet->data for function 1 & 2 is actually the number of boolean points
  unsigned char no_of_registers = packet->data / 16;
  unsigned char number_of_bytes = no_of_registers * 2; 
       
  // if the number of points dont fit in even 2byte amounts (one register) then use another register and pad 
  if (packet->data % 16 > 0) 
  {
    no_of_registers++;
    number_of_bytes++;
  }
             
  if (frame[2] == number_of_bytes) // check number of bytes returned
  { 
    unsigned char bytes_processed = 0;
    unsigned char index = 3; // start at the 4th element in the frame and combine the Lo byte  
    unsigned int temp;
    for (unsigned char i = 0; i < no_of_registers; i++)
    {
      temp = frame[index]; 
      bytes_processed++;
      if (bytes_processed < number_of_bytes)
      {
				temp = (frame[index + 1] << 8) | temp;
        bytes_processed++;
        index += 2;
      }
      register_array[packet->local_start_address + i] = temp;
    }
    processSuccess(); 
  }
  else // incorrect number of bytes returned 
    processError();
}

void process_F3_F4()
{
	// check number of bytes returned - unsigned int == 2 bytes
  // data for function 3 & 4 is the number of 
  //@pepsilla:
  //unsigned char no_of_change_flags = sizeof (register_change_flags);
  unsigned char i;
  //Reset register_change_flags array to check old_value and new_value
  //for (i=0;i<no_of_change_flags)register_change_flags[i]=0;
  packet->change_flags = false; //Reset changes in packet from slave; 
  
  if (frame[2] == (packet->data * 2)) 
  {
    unsigned char index = 3;
    unsigned int new_value;
    unsigned int old_value;
    unsigned char register_flag;
    unsigned int local_addres;
    unsigned char register_bit;
    unsigned char value;
    
    for (i = 0; i < packet->data; i++)
    {
      // start at the 4th element in the frame and combine the Lo byte 
      old_value = register_array[packet->local_start_address + i];
      new_value = (frame[index] << 8) | frame[index + 1];
      if(old_value != new_value){
		local_addres = packet->local_start_address + i;
		register_array[local_addres] = new_value;
		//Set change_flag_matrix:
		packet->change_flags = true; //Changes in packet
		//register_flag is an array of unsigned char (one memory position of arduino RAM),
		//Each bite of change_flags represent a change in register_array from 0 to sizeof(register_array) 
		register_flag = local_addres/16;
		register_bit = local_addres % 16;
		value = 1;
		value <<= (15-register_bit);
		register_change_flags[register_flag] |= value;
	  }
	  index += 2;
    }
    processSuccess(); 
  }
  else // incorrect number of bytes returned  
    processError();  
}

void process_F5_F6_F15_F16()
{
	// The repsonse of functions 5,6,15 & 16 are just an echo of the query
  unsigned int recieved_address = ((frame[2] << 8) | frame[3]);
  unsigned int recieved_data = ((frame[4] << 8) | frame[5]);
		
  if ((recieved_address == packet->address) && (recieved_data == packet->data))
    processSuccess();
  else
    processError();
}

void processError()
{
	packet->retries++;
	packet->failed_requests++;
	
	// if the number of retries have reached the max number of retries 
  // allowable, stop requesting the specific packet
  if (packet->retries == retry_count)
	{
    packet->connection = 0;
		packet->retries = 0;
	}
	state = WAITING_FOR_TURNAROUND;
	delayStart = millis(); // start the turnaround delay
	is_update=false;
}

void processSuccess()
{
	packet->successful_requests++; // transaction sent successfully
	packet->retries = 0; // if a request was successful reset the retry counter
	state = WAITING_FOR_TURNAROUND;
	delayStart = millis(); // start the turnaround delay
}
  
void modbus_configure(HardwareSerial* SerialPort,
											long baud,
											unsigned char byteFormat,
											long _timeout, 
											long _polling, 
											unsigned char _retry_count, 
											unsigned char _TxEnablePin, 
											Packet* _packets, 
											unsigned int _total_no_of_packets,
											unsigned int* _register_array,
											unsigned int* _register_change_flags)
{ 
	// Modbus states that a baud rate higher than 19200 must use a fixed 750 us 
  // for inter character time out and 1.75 ms for a frame delay for baud rates
  // below 19200 the timing is more critical and has to be calculated.
  // E.g. 9600 baud in a 11 bit packet is 9600/11 = 872 characters per second
  // In milliseconds this will be 872 characters per 1000ms. So for 1 character
  // 1000ms/872 characters is 1.14583ms per character and finally modbus states
  // an inter-character must be 1.5T or 1.5 times longer than a character. Thus
  // 1.5T = 1.14583ms * 1.5 = 1.71875ms. A frame delay is 3.5T.
	// Thus the formula is T1.5(us) = (1000ms * 1000(us) * 1.5 * 11bits)/baud
	// 1000ms * 1000(us) * 1.5 * 11bits = 16500000 can be calculated as a constant
	
	if (baud > 19200)
		T1_5 = 750; 
	else 
		T1_5 = 16500000/baud; // 1T * 1.5 = T1.5
		
	/* The modbus definition of a frame delay is a waiting period of 3.5 character times
		 between packets. This is not quite the same as the frameDelay implemented in
		 this library but does benifit from it.
		 The frameDelay variable is mainly used to ensure that the last character is 
		 transmitted without truncation. A value of 2 character times is chosen which
		 should suffice without holding the bus line high for too long.*/
		 
	frameDelay = T1_5 * 2; 
	
	// initialize
	state = IDLE;
    timeout = _timeout;
    polling = _polling;
	retry_count = _retry_count;
	TxEnablePin = _TxEnablePin;
	total_no_of_packets = _total_no_of_packets;
	packetArray = _packets;
	register_array = _register_array;
	register_change_flags =_register_change_flags;
	ModbusPort = SerialPort;
	(*ModbusPort).begin(baud, byteFormat);
	
	pinMode(TxEnablePin, OUTPUT);
  digitalWrite(TxEnablePin, LOW);
	
} 

void modbus_construct(Packet *_packet, 
											unsigned char id, 
											unsigned char function, 
											unsigned int address, 
											unsigned int data,
											unsigned int local_start_address)
{
	_packet->id = id;
  _packet->function = function;
  _packet->address = address;
  _packet->data = data;
	_packet->local_start_address = local_start_address;
	_packet->connection = 1;
}

unsigned int calculateCRC(unsigned char bufferSize) 
{
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (unsigned char i = 0; i < bufferSize; i++)
  {
    temp = temp ^ frame[i];
    for (unsigned char j = 1; j <= 8; j++)
    {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)
        temp ^= 0xA001;
    }
  }
  // Reverse byte order. 
  temp2 = temp >> 8;
  temp = (temp << 8) | temp2;
  temp &= 0xFFFF;
  // the returned value is already swapped
  // crcLo byte is first & crcHi byte is last
  return temp; 
}

void sendPacket(unsigned char bufferSize)
{
	unsigned char lowByte;
	unsigned char higByte;
	unsigned char lrcByte;
	int lrcCheck = 0;
	
	digitalWrite(TxEnablePin, HIGH);

  //Out Init ASCII Modbus Frame :byte|unsigned char
  (*ModbusPort).write(':');
  if(_debugMode)Serial.write('>');
  for (unsigned char i = 0; i < bufferSize; i++)
  {
    //Two ASCII bytes to One Byte Modbus frame
    lrcCheck += frame[i];
    lowByte = dec_to_ASCII_HEX(frame[i]);
    higByte = dec_to_ASCII_HEX(frame[i] >> 4);
    (*ModbusPort).write(char(higByte));
    if(_debugMode)Serial.write(char(higByte));
    (*ModbusPort).write(char(lowByte));
    if(_debugMode)Serial.write(char(lowByte));
  }
  // Calculate  and send LRC
  lrcCheck = lrcCheck*-1;
  lrcByte = lrcCheck >> 8;
  lowByte = dec_to_ASCII_HEX(lrcByte);
  higByte = dec_to_ASCII_HEX(lrcByte >> 4);
  (*ModbusPort).write(char(higByte));
  if(_debugMode)Serial.write(char(higByte));
  (*ModbusPort).write(char(lowByte));
  if(_debugMode)Serial.write(char(lowByte));
  lrcByte = lrcCheck & 0xFF;
  lowByte = dec_to_ASCII_HEX(lrcByte);
  higByte = dec_to_ASCII_HEX(lrcByte >> 4);
  (*ModbusPort).write(char(higByte));
  if(_debugMode)Serial.write(char(higByte));
  (*ModbusPort).write(char(lowByte));
  if(_debugMode)Serial.write(char(lowByte));
  
  (*ModbusPort).write('\r');
  if(_debugMode)Serial.write('\r');
  (*ModbusPort).write('\n');
  if(_debugMode)Serial.write('\n');
  (*ModbusPort).flush();
  Serial.flush();  
    
    // allow a frame delay to indicate end of transmission
    //delayMicroseconds(T3_5); 
    
  digitalWrite(TxEnablePin, LOW);
  delayStart = millis(); // start the timeout delay	
  t_getPacket = millis();
}

// Conversion from unsigned char BCD [0,1,2...,16]
//            to unsigned cahr ASCII base 16.
unsigned char dec_to_ASCII_HEX(unsigned char decChar){
	//Discard left side of BCD
	decChar &= B00001111;
	if(decChar<10){
		//ASCII [0,1,2...9]
		decChar += 48;
		
	}
	else{
		//ASCII[A,B,...F]
		decChar += 55;
	}
	return decChar;
}

long get_timeout(){
	return timeout;
}
long get_polling(){
	return polling;
}
unsigned char get_retry_count(){
	return retry_count;
}
void set_timeout(long value){
	timeout = value;
}
void set_polling(long value){
	polling = value;
}
void set_retry_count(unsigned char value){
	retry_count = value;
}
