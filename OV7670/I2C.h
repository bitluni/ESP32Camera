#include "Arduino.h"

template<int SDA, int SCL>
class I2C
{
  static void inline DELAY()
  {
    delayMicroseconds(1);
  }

  static void inline SCLLOW()
  {
    pinMode(SCL, OUTPUT);
    digitalWrite(SCL, 0);
  }

  static void inline SCLHIGH()
  {
    pinMode(SCL, INPUT_PULLUP);  
    digitalWrite(SCL, 1);
  }

  static void inline CLOCK()
  {
    DELAY();
    SCLHIGH();
    DELAY();
    DELAY();
    SCLLOW();
    DELAY();
  }
  
  static void inline SDALOW()
  {
    pinMode(SDA, OUTPUT);
    digitalWrite(SDA, 0);  
  }
  
  static void inline SDAHIGH()
  {
    pinMode(SDA, OUTPUT);
    digitalWrite(SDA, 1);  
  }

  static void inline SDAPULLUP()
  {
    pinMode(SDA, INPUT_PULLUP);  
  }

  static void pushByte(unsigned char b)
  {
    for(char i = 0; i < 8; i++)
    {
      if(b & 0x80)
        SDAHIGH();
      else
        SDALOW();
      b <<= 1;
      CLOCK();
    }  
  }
  
  static bool getAck()
  {
    SDAPULLUP();
    DELAY();
    SCLHIGH();
    DELAY();
    int r = digitalRead(SDA);
    SDALOW();
    DELAY();
    SCLLOW();
    DELAY();
    return r == 0;
  }

  static void start()
  {
    SDAPULLUP();
    DELAY();
    SCLHIGH();
    DELAY();
    SDALOW();
    DELAY();
    SCLLOW();
    DELAY();
  }
  
  static void end()
  {
    SCLHIGH();
    DELAY();
    SDAPULLUP();
    DELAY();
  }

  public:
  static void init()
  {
    pinMode(SDA, INPUT_PULLUP);
    pinMode(SCL, INPUT_PULLUP);
    digitalWrite(SDA, 0);
    digitalWrite(SCL, 0);
  }
  
  static bool writeRegister(unsigned char addr, unsigned char reg, unsigned char data)
  {
    start();
    pushByte(addr);
    
    if(!getAck())
    {
      end();
      return false;
    }
    
    pushByte(reg);
    if(!getAck())
    {
      end();
      return false;
    }
  
    pushByte(data);
    if(!getAck())
    {
      end();
      return false;
    }
  
    end();
    return true;
  }
};
