/*
 * I2CPCF8591.cpp
 *
 *  Created on: 12 ago. 2016
 *      Author: JoseMaria
 */
#include "Wire.h"
#include <I2CPCF8591.h>

I2CPCF8591::I2CPCF8591(int address, uint8_t controlByte){
	_address = address;
	_controlByte = controlByte;

	_analogInput[0] = 0;
	_analogInput[1] = 0;
	_analogInput[2] = 0;
	_analogInput[3] = 0;

	_analogOutput = 0;
	_lastError = false;
}

I2CPCF8591::I2CPCF8591(){
	_address = 0x20;
	_controlByte = B11110000;

	_analogInput[0] = 0;
	_analogInput[1] = 0;
	_analogInput[2] = 0;
	_analogInput[3] = 0;

	_analogOutput = 0;
	_lastError = false;
}

uint8_t I2CPCF8591::initPort(){
	// Set Inputs on PCF8591 from defined control byte
	// Initialize  analog output
	// I2C write operations to PCF8591
	Wire.beginTransmission(_address);
	Wire.write(_controlByte); //Chip configuration.
	//If analog output is enabled set to 0 analog output port.
	uint8_t analogOutput_Enebled = _controlByte & B01000000;
	if( analogOutput_Enebled)
		Wire.write(_analogOutput);// Send 0 default value on init
	Wire.endTransmission();

	//Initialize Analog inputs;
	readPort();

	return 0;
}

uint8_t I2CPCF8591::readPort(){
	uint8_t inputsMode = _controlByte;
	unsigned int counter = 0;
	inputsMode >>= 4; //Shift right four bits;
	inputsMode &= 3; //Only four modes for input configuration.

	_controlByte |= B00000100; //Sets auto increment on read

	//case 0: Four single ended inputs. Page 6 of NPX DataSheet.
	if(inputsMode == 0) counter = 4;
	//case 1:three diferential inputs
	//case 2:single ended and  diferential mixed
	//case 3:two  diferential inputs
	else counter = 3;

	Wire.beginTransmission(_address);
	Wire.write(_controlByte);
	Wire.endTransmission();

	Wire.requestFrom(_address,counter+1); // Request data from chip
	Wire.read(); //Discard previous read

	for (uint8_t n = 0; n<counter;n++){
		_analogInput[n] = Wire.read();
	}

	return 0;
}



uint8_t I2CPCF8591::getAnalogValue(uint8_t chanel){
	if(chanel>=0 && chanel<4)
		return(_analogInput[chanel]);
	if(chanel == 4)
		return _analogOutput;
	return 0xff;
}

uint8_t I2CPCF8591::setAnalogValue(uint8_t value){
	uint8_t analogOutput_Enebled = _controlByte & B01000000;
	if( analogOutput_Enebled){
		Wire.beginTransmission(_address);
		Wire.write(_controlByte);
		Wire.write(value);
		Wire.endTransmission();
		_analogOutput = value;
		return value;
	}
	return 0xff;
}

boolean I2CPCF8591::lastError(){
	return _lastError;
}

uint8_t I2CPCF8591::getControlByte(){
	return _controlByte;
}
