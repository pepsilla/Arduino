/*
 * I2CRX2002ABIWTS.h
 *
 *  Created on: 12 ago. 2016
 *      Author: JoseMaria
 */

#ifndef LIBRARIES_I2CRX2002ABIWTS_I2CRX2002ABIWTS_H_
#define LIBRARIES_I2CRX2002ABIWTS_I2CRX2002ABIWTS_H_
#include "Arduino.h"

#define I2CRX2002ABIWTS_ADDRESS 0x3c
#define I2CRX2002ABIWTS_MAXCHAR 20

class I2CRX2002ABIWTS
{
	public:
		I2CRX2002ABIWTS();
		I2CRX2002ABIWTS(int address);
		uint8_t init();
		uint8_t clear();
		uint8_t goHome();
		uint8_t printLine1(String line1);
		uint8_t printLine2(String line2);
		uint8_t setLine1(String line1);
		uint8_t setLine2(String line2);
		uint8_t print();
	private:
		int _address;
		uint8_t _line1[I2CRX2002ABIWTS_MAXCHAR+1];
		uint8_t _line2[I2CRX2002ABIWTS_MAXCHAR+1];
};



#endif /* LIBRARIES_I2CRX2002ABIWTS_I2CRX2002ABIWTS_H_ */
