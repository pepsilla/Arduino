/*
 * I2CPCF8591.h
 *
 *  Created on: 12 ago. 2016
 *      Author: JoseMaria
 */

#ifndef LIBRARIES_I2CPCF8591_I2CPCF8591_H_
#define LIBRARIES_I2CPCF8591_I2CPCF8591_H_
#include "Arduino.h"

#define I2CPCF8591_ADDRESS 0xD0
#define I2CPCF8591_CONTROLBYTE B01000100

class I2CPCF8591
{
	public:
		I2CPCF8591();
		I2CPCF8591(int address, uint8_t controlByte);
		uint8_t getAnalogValue(uint8_t chanel);
		uint8_t setAnalogValue(uint8_t value);
		boolean lastError();
		uint8_t readPort();
		uint8_t writePort();
		uint8_t initPort();
		uint8_t getControlByte();
	private:
		int _address;
		uint8_t _analogInput[4];
		uint8_t _analogOutput;
		uint8_t _controlByte;
		boolean _lastError;
};



#endif /* LIBRARIES_I2CPCF8591_I2CPCF8591_H_ */
