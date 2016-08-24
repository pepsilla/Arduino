/*
 *  i2ckeypad.pde - keypad/I2C expander interface example for Arduino
 *
 *  Copyright (c) 2009 Angel Sancho <angelitodeb@gmail.com>
 *  All rights reserved.
 *
 *
 *  LICENSE
 *  -------
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  EXPLANATION
 *  -----------
 *  This example was tested with PCF8574P and SPARKFUN 4x3 keypad wired
 *  as you can see in the image i2ckeypad_example_schema.png, and with
 *  default pin mapping of i2ckeypad library
 *
 *  R1, R2 and R3 are 10K
 */

#include <Wire.h>
#include <i2ckeypad.h>

#define ROWS 4
#define COLS 3

// With A0, A1 and A2 of PCF8574 to ground I2C address is 0x20
#define PCF8574_ADDR 0x20


i2ckeypad kpd = i2ckeypad(PCF8574_ADDR, ROWS, COLS);

void setup()
{
  Serial.begin(9600);

  Wire.begin();

  kpd.init();

  Serial.print("Testing keypad/PCF8574 I2C port expander arduino lib\n\n");
}

void loop()
{
  char key = kpd.get_key();

  if(key != '\0') {
        Serial.print(key);
  }
}

