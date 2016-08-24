/*
 * I2CPCF8574.h
 *
 *  Created on: 12 ago. 2016
 *      Author: Jose Maria. @pepsilla
 *
 */

#ifndef LIBRARIES_I2CPCF8574_I2CPCF8574_H_
#define LIBRARIES_I2CPCF8574_I2CPCF8574_H_
#include "Arduino.h"

#define I2CPCF8574_ADDRESS 0x20
#define I2CPCF8574_IOMASK 0b11110000

class I2CPCF8574
{
	public:
		I2CPCF8574();
		I2CPCF8574(int address, uint8_t ioMask);
		uint8_t getPinPort(uint8_t bite);
		uint8_t setPinPort(uint8_t bite, uint8_t value);
		boolean lastError();
		uint8_t readPort();
		uint8_t writePort();
		uint8_t initPort();
		uint8_t getIoMask();
	private:
		int _address;
		uint8_t _inputFlag;
		uint8_t _outputFlag;
		uint8_t _ioMask;
		boolean _lastError;
};



#endif /* LIBRARIES_I2CPCF8574_I2CPCF8574_H_ */
