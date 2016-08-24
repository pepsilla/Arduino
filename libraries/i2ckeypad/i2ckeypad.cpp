/*
 *  i2ckeypad.cpp v0.1 - keypad/I2C expander interface for Arduino
 *
 *  Copyright (c) 2009 Angel Sancho <angelitodeb@gmail.com>
 *  All rights reserved.
 *
 *  Original source from keypad v0.3 of Mark Stanley <mstanley@technologist.com>
 *  (http://www.arduino.cc/playground/Main/KeypadTutorial)
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
 *  This library is designed for use with PCF8574, but can possibly be
 *  adapted to other I2C port expanders
 *
 *  Wiring diagrams for PCF8574 and 4x3 keypad can be found under
 *  examples directory. Library runs correctly without cols pull-up
 *  resistors but it's better to use it
 *
 *  You can change pin connections between PCF8574 and keypad under
 *  PIN MAPPING section below
 *
 *  IMPORTANT! You have to call Wire.begin() before init() in your code
 *
 *  ... and sorry for my poor english!
 */

#include "i2ckeypad.h"
#include <Wire.h>

/*extern "C" {
  #include "WConstants.h"
}*/


/*
 *  PIN MAPPING
 *
 *  Here you can change your wire mapping between your keypad and PCF8574
 *  Default mapping is for sparkfun 4x3 keypad
 */

#define COL0  0  // P2 of PCF8574, col0 is usually pin 3 of 4x3 keypads
#define COL1  1  // P0 of PCF8574, col1 is usually pin 1 of 4x3 keypads
#define COL2  2  // P4 of PCF8574, col2 is usually pin 5 of 4x3 keypads
#define COL3  3  // sorry, don't have a 4x4 keypad to try it
#define ROW0  4  // P1 of PCF8574, row0 is usually pin 2 of 4x3 keypads
#define ROW1  5  // P6 of PCF8574, row1 is usually pin 7 of 4x3 keypads
#define ROW2  6  // P5 of PCF8574, row2 is usually pin 6 of 4x3 keypads
#define ROW3  7  // P3 of PCF8574, row3 is usually pin 4 of 4x3 keypads


/*
 *  KEYPAD KEY MAPPING
 *
 *  Default key mapping for 4x4 keypads, you can change it here if you have or
 *  like different keys
 */

const char keymap[4][5] =
{
  "123A",
  "456B",
  "789C",
  "*0#D"
};


/*
 *  VAR AND CONSTANTS DEFINITION. Don't change nothing here
 *
 */

// Default row and col pin counts
int num_rows = 4;
int num_cols = 3;

// PCF8574 i2c address
int pcf8574_i2c_addr;

// Current search row
static int row_select;

// Current data set in PCF8574
static int current_data;

// Hex byte statement for each port of PCF8574
const int hex_data[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

// Hex data for each row of keypad in PCF8574
const int pcf8574_row_data[4] = 
{
  hex_data[ROW1] | hex_data[ROW2] | hex_data[ROW3] |
  hex_data[COL0] | hex_data[COL1] | hex_data[COL2] | hex_data[COL3],
  hex_data[ROW0] | hex_data[ROW2] | hex_data[ROW3] |
  hex_data[COL0] | hex_data[COL1] | hex_data[COL2] | hex_data[COL3],
  hex_data[ROW0] | hex_data[ROW1] | hex_data[ROW3] |
  hex_data[COL0] | hex_data[COL1] | hex_data[COL2] | hex_data[COL3],
  hex_data[ROW0] | hex_data[ROW1] | hex_data[ROW2] |
  hex_data[COL0] | hex_data[COL1] | hex_data[COL2] | hex_data[COL3],
};

// Hex data for each col of keypad in PCF8574
int col[4] = {hex_data[COL0], hex_data[COL1], hex_data[COL2], hex_data[COL3]};


/*
 *  CONSTRUCTORS
 */

i2ckeypad::i2ckeypad(int addr)
{
  pcf8574_i2c_addr = addr;
}

i2ckeypad::i2ckeypad(int addr, int r, int c)
{
  pcf8574_i2c_addr = addr;
  num_rows = r;
  num_cols = c;
}


/*
 *  PUBLIC METHODS
 */

void i2ckeypad::init()
{
  // All PCF8574 ports high
  pcf8574_write(pcf8574_i2c_addr, 0xff);

  // Start with the first row
  row_select = 0;
}

char i2ckeypad::get_key()
{
  static int temp_key;

  int tmp_data;
  int r;

  int key = '\0';

  // Search row low
  pcf8574_write(pcf8574_i2c_addr, pcf8574_row_data[row_select]);

  for(r=0;r<num_cols;r++) {
    // Read pcf8574 port data
    tmp_data = pcf8574_byte_read(pcf8574_i2c_addr);

    // XOR to compare obtained data and current data and know
    // if some column are low
    tmp_data ^= current_data;

    // Key pressed!
    if(col[r] == tmp_data) {
      temp_key = keymap[row_select][r];
      return '\0';
    }
  }

  // Key was pressed and then released
  if((key == '\0') && (temp_key != '\0'))    
  {
    key = temp_key;
    temp_key = '\0';
    return key;
  }

  // All PCF8574 ports high again
  pcf8574_write(pcf8574_i2c_addr, 0xff);

  // Next row
  row_select++;
  if(row_select == num_rows) {
    row_select = 0;
  }

  return key;
}

/*
 *  PRIVATE METHODS
 */

void i2ckeypad::pcf8574_write(int addr, int data)
{
  current_data = data;

  Wire.beginTransmission(addr);
  Wire.write(data);
  Wire.endTransmission();
}

int i2ckeypad::pcf8574_byte_read(int addr)
{
  Wire.requestFrom(addr, 1);

  return Wire.read();
}

