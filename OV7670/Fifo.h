#include "Arduino.h"

template<int RRST, int WRST, int RCK, int WR, int D0, int D1, int D2, int D3, int D4, int D5, int D6, int D7>
class Fifo
{
  public:
  
  static void inline init()
  {
    pinMode(RRST, OUTPUT);
    pinMode(RCK, OUTPUT);
    pinMode(WRST, OUTPUT);
    pinMode(WR, OUTPUT);
    digitalWrite(RRST, 1);
    digitalWrite(WRST, 1);
    digitalWrite(WR, 1);
    pinMode(D0, INPUT);
    pinMode(D1, INPUT);
    pinMode(D2, INPUT);
    pinMode(D3, INPUT);
    pinMode(D4, INPUT);
    pinMode(D5, INPUT);
    pinMode(D6, INPUT);
    pinMode(D7, INPUT);
  }

  static void inline readReset()
  {
      digitalWrite(RRST, 0);
      delayMicroseconds(1);
      digitalWrite(RCK, 0);
      delayMicroseconds(1);
      digitalWrite(RCK, 1);
      delayMicroseconds(1);
      digitalWrite(RRST, 1);  
  }

  static void inline writeEnable()
  {
      digitalWrite(WR, 0);
  }
  
  static void inline writeDisable()
  {
      digitalWrite(WR, 1);
  }

  static void inline writeReset()
  {
      digitalWrite(WRST, 0);
      delayMicroseconds(1);
      digitalWrite(WRST, 1);
  }

  static unsigned char inline readByte()
  {
      digitalWrite(RCK, 1);
      unsigned char b = 
        digitalRead(D0) | 
        (digitalRead(D1) << 1) | 
        (digitalRead(D2) << 2) | 
        (digitalRead(D3) << 3) | 
        (digitalRead(D4) << 4) | 
        (digitalRead(D5) << 5) | 
        (digitalRead(D6) << 6) | 
        (digitalRead(D7) << 7); 
      digitalWrite(RCK, 0);
      return b;
  }

  static void inline skipByte()
  {
      digitalWrite(RCK, 1);
      digitalWrite(RCK, 1); //just for the delay
      digitalWrite(RCK, 0);
      digitalWrite(RCK, 0); //needed if more skips in sequence
  }
  static void inline readBytes(unsigned char *buffer, int count)
  {
    for(int i = 0; i < count; i++)
      buffer[i] = readByte();
  }
};

