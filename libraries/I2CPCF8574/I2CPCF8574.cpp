/*
 * I2CPCF8574.cpp
 *
 *  Created on: 12 ago. 2016
 *      Author: JoseMaria
 */
#include "Wire.h"
#include <I2CPCF8574.h>

I2CPCF8574::I2CPCF8574(int address, uint8_t ioMask){
	_address = address;
	_ioMask = ioMask;
	_inputFlag = 0;
	_outputFlag = 0;
	_lastError = false;
}

I2CPCF8574::I2CPCF8574(){
	_address = I2CPCF8574_ADDRESS;
	_ioMask = I2CPCF8574_IOMASK;
	_inputFlag = 0;
	_outputFlag = 0;
	_lastError = false;
}

uint8_t I2CPCF8574::initPort(){
	//Input pins are set their bite to 1 in _ioMask.
	_outputFlag = 0xff; //default value on power-on chip.
	Wire.beginTransmission(_address);
	Wire.write(_outputFlag);
	Wire.endTransmission();

	Wire.requestFrom(_address,1,0); // refresh _inputFlag.
	_inputFlag = Wire.read();

	return 0;
}

uint8_t I2CPCF8574::readPort(){
	Wire.requestFrom(_address,1); // Pedir a la direccion DIRECCION_PCF8574 1 byte
	if(Wire.available() == 1) _inputFlag = Wire.read();
	else{
		//printAgroDisplay_Line1("* ERROR I/O EXPANDER");
	}
	return 0;
}

uint8_t I2CPCF8574::writePort(){
	Wire.beginTransmission(_address);
	Wire.write(_ioMask | _outputFlag); //All ports are inputs and oputput ports set to HIGH state.
	Wire.endTransmission();
	return 0;
}

uint8_t I2CPCF8574::getPinPort(uint8_t bite){
	if(bite>=0 && bite<8){
		uint8_t latch = _inputFlag;
		latch >>= bite;
		latch &= 1;
		/*Serial.println();
		Serial.print("byte: ");
		Serial.print(bite);
		Serial.print(", value: ");
		Serial.println(_inputFlag,2);*/
		return latch;
	}
	return 0xff;
}

uint8_t I2CPCF8574::setPinPort(uint8_t bite, uint8_t value){
	if(bite>=0 && bite<8){
		uint8_t mask = 1;
		mask <<= bite;
		if(value) _outputFlag |= mask;
		else _outputFlag &= ~mask;
	}
	return _outputFlag;
}

boolean I2CPCF8574::lastError(){
	return _lastError;
}

uint8_t I2CPCF8574::getIoMask(){
	return _ioMask;
}
