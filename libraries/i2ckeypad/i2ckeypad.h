#ifndef i2ckeypad_h
#define i2ckeypad_h

#include <inttypes.h>

class i2ckeypad {
public:
  i2ckeypad(int);
  i2ckeypad(int, int, int);
  char get_key();
  void init();
  
private:
  void pcf8574_write(int, int);
  int pcf8574_byte_read(int);
};

#endif

