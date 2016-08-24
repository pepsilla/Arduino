/*
 * I2CRX2002ABIWTS.cpp
 *
 *  Created on: 12 ago. 2016
 *      Author: JoseMaria
 */
#include "Wire.h"
#include <I2CRX2002ABIWTS.h>

I2CRX2002ABIWTS::I2CRX2002ABIWTS(int address){
	_address = address;
}

I2CRX2002ABIWTS::I2CRX2002ABIWTS(){
	_address = I2CRX2002ABIWTS_ADDRESS;
}

uint8_t I2CRX2002ABIWTS::init(){
	Wire.beginTransmission(_address);
	Wire.write(0x00);
	Wire.write(0x38);
	Wire.endTransmission();
	Wire.beginTransmission(_address);
	Wire.write(0x80);
	Wire.write(0x39);
	Wire.write(0x14);
	Wire.write(0x74);
	Wire.write(0x54);
	Wire.write(0x6f);
	Wire.write(0x0c);
	Wire.write(0x01);
	Wire.endTransmission();
	delay(2);
	_line1[0] = 0;
	_line2[0] = 0;

	return 0;
}

uint8_t I2CRX2002ABIWTS::clear(){
	Wire.beginTransmission(_address);
	Wire.write(0);
	Wire.write(1);
	Wire.endTransmission();
	delay(2);
	return 0;
}

uint8_t I2CRX2002ABIWTS::goHome(){
	Wire.beginTransmission(_address);
	Wire.write(0);
	Wire.write(2);
	Wire.endTransmission();
	delay(2);
	return 0;
}

uint8_t I2CRX2002ABIWTS::printLine1(String line1){
	unsigned int contador;
	unsigned int lenLine1 = line1.length();
	if(lenLine1>0){
	  Wire.beginTransmission(_address);
	  Wire.write(0x80);
	  Wire.write(0x80); //For line1
	  Wire.write(0x40);

	  contador = 0;
	  while(contador<I2CRX2002ABIWTS_MAXCHAR && contador<lenLine1)
	  {
		  _line1[contador] = line1.charAt(contador);
		  contador++;
	  }
	  while (contador<I2CRX2002ABIWTS_MAXCHAR){
		  _line1[contador] = ' ';
		  contador++;
	  }
	  _line1[contador] = 0;
	  for(contador = 0; contador<I2CRX2002ABIWTS_MAXCHAR;contador++)
	    Wire.write(_line1[contador]);
	  Wire.endTransmission();
	  return 0x00;
	}
	return 0xff;
}

uint8_t I2CRX2002ABIWTS::printLine2(String line2){
	unsigned int contador;
	unsigned int lenLine2 = line2.length();
	if(lenLine2>0){
		Wire.beginTransmission(_address);
		Wire.write(0x80);
		Wire.write(0xc0); //For line2
		Wire.write(0x40);

		contador = 0;
		while(contador<I2CRX2002ABIWTS_MAXCHAR && contador<lenLine2)
		{
			_line2[contador] = line2.charAt(contador);
			contador++;
		}
		while (contador<I2CRX2002ABIWTS_MAXCHAR){
			_line2[contador] = ' ';
			contador++;
		}
		_line2[contador] = 0;
		for(contador = 0; contador<I2CRX2002ABIWTS_MAXCHAR;contador++)
			Wire.write(_line2[contador]);
		Wire.endTransmission();
		return 0x00;
	}
	return 0xff;
}
uint8_t I2CRX2002ABIWTS::setLine1(String line1){
	unsigned int contador = 0;
	unsigned int lenLine1 = line1.length();
	if(lenLine1>0){
		contador = 0;
		while(contador<I2CRX2002ABIWTS_MAXCHAR && contador<lenLine1)
		{
			_line1[contador] = line1.charAt(contador);
			contador++;
		}
		while (contador<I2CRX2002ABIWTS_MAXCHAR){
			_line1[contador] = ' ';
			contador++;
		}
		_line1[contador] = 0;
	  return 0x00;
	}
	return 0xff;
}

uint8_t I2CRX2002ABIWTS::setLine2(String line2){
	unsigned int contador = 0;
	unsigned int lenLine2 = line2.length();
	if(lenLine2>0){
		contador = 0;
		while(contador<I2CRX2002ABIWTS_MAXCHAR && contador<lenLine2)
		{
			_line2[contador] = line2.charAt(contador);
			contador++;
		}
		while (contador<I2CRX2002ABIWTS_MAXCHAR){
			_line2[contador] = ' ';
			contador++;
		}
		_line2[contador] = 0;
	  return 0x00;
	}
	return 0xff;
}
uint8_t I2CRX2002ABIWTS::print(){
	unsigned int contador = 0;

	Wire.beginTransmission(_address);
	Wire.write(0x80);
	Wire.write(0x80); //For line1
	Wire.write(0x40);

	for(contador = 0; contador<I2CRX2002ABIWTS_MAXCHAR;contador++)
		Wire.write(_line1[contador]);

	Wire.endTransmission();

	Wire.beginTransmission(_address);
	Wire.write(0x80);
	Wire.write(0xc0); //For line2
	Wire.write(0x40);

	for(contador = 0; contador<I2CRX2002ABIWTS_MAXCHAR;contador++)
			Wire.write(_line2[contador]);

	Wire.endTransmission();
	return 0xff;
}
