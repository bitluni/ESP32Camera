#include "Arduino.h"
#include "Fifo.h"

template<class I2C, int RRST, int WRST, int RCK, int WR, int D0, int D1, int D2, int D3, int D4, int D5, int D6, int D7>
class FifoCamera
{
  static const int ADDR = 0x42;

  static const int REG_GAIN = 0x00;
  static const int REG_BLUE = 0x01;
  static const int REG_RED = 0x02;
  static const int REG_COM1 = 0x04;
  static const int REG_VREF = 0x03;
  static const int REG_COM4 = 0x0d;
  static const int REG_COM5 = 0x0e;
  static const int REG_COM6 = 0x0f;
  static const int REG_AECH = 0x10;
  static const int REG_CLKRC = 0x11;
  static const int REG_COM7 = 0x12;
    static const int COM7_RGB = 0x04;
  static const int REG_COM8 = 0x13;
    static const int COM8_FASTAEC = 0x80;    // Enable fast AGC/AEC
    static const int COM8_AECSTEP = 0x40;    // Unlimited AEC step size
    static const int COM8_BFILT = 0x20;    // Band filter enable
    static const int COM8_AGC = 0x04;    // Auto gain enable
    static const int COM8_AWB = 0x02;    // White balance enable
    static const int COM8_AEC = 0x0;
  static const int REG_COM9 = 0x14;
  static const int REG_COM10 = 0x15;
  static const int REG_COM14 = 0x3E;
  static const int REG_COM11 = 0x3B;
  static const int COM11_NIGHT = 0x80;
  static const int COM11_NMFR = 0x60;
  static const int COM11_HZAUTO = 0x10;
  static const int COM11_50HZ = 0x08;
  static const int COM11_EXP = 0x0;
  static const int REG_TSLB = 0x3A;
  static const int REG_RGB444 = 0x8C;
  static const int REG_COM15 = 0x40;
    static const int COM15_RGB565 = 0x10;
    static const int COM15_R00FF = 0xc0;
  static const int REG_HSTART = 0x17;
  static const int REG_HSTOP = 0x18;
  static const int REG_HREF = 0x32;
  static const int REG_VSTART = 0x19;
  static const int REG_VSTOP = 0x1A;
  static const int REG_COM3 = 0x0C;
  static const int REG_MVFP = 0x1E;
  static const int REG_COM13 = 0x3d;
    static const int COM13_UVSAT = 0x40;
  static const int REG_SCALING_XSC = 0x70;
  static const int REG_SCALING_YSC = 0x71;    
  static const int REG_SCALING_DCWCTR = 0x72;
  static const int REG_SCALING_PCLK_DIV = 0x73;
  static const int REG_SCALING_PCLK_DELAY = 0xa2;
  static const int REG_BD50MAX = 0xa5;
  static const int REG_BD60MAX = 0xab;
  static const int REG_AEW = 0x24;
  static const int REG_AEB = 0x25;
  static const int REG_VPT = 0x26;
  static const int REG_HAECC1 = 0x9f;
  static const int REG_HAECC2 = 0xa0;
  static const int REG_HAECC3 = 0xa6;
  static const int REG_HAECC4 = 0xa7;
  static const int REG_HAECC5 = 0xa8;
  static const int REG_HAECC6 = 0xa9;
  static const int REG_HAECC7 = 0xaa;
  static const int REG_COM12 = 0x3c;
  static const int REG_GFIX = 0x69;
  static const int REG_COM16 = 0x41;
  static const int COM16_AWBGAIN = 0x08;
  static const int REG_EDGE = 0x3f;
  static const int REG_REG76 = 0x76;
  static const int ADCCTR0 = 0x20;

  I2C &i2c;
  Fifo<RRST, WRST, RCK, WR, D0, D1, D2, D3, D4, D5, D6, D7> fifo;
  
  public:
  FifoCamera(I2C &_i2c)
    :i2c(_i2c)
  { 
  }

  void init()
  {
    fifo.init();
    
    //reset all registers to default
    i2c.writeRegister(ADDR, REG_COM7, 0x80);
  }

  void testImage()
  {
    //i2c.writeRegister(ADDR, 0x70, 0x4A | 0x80);
    i2c.writeRegister(ADDR, 0x71, 0x35 | 0x80);
  }

  void prepareCapture()
  {
    fifo.writeReset();
  }

  void startCapture()
  {
    fifo.writeEnable();
  }
  
  void stopCapture()
  {
    fifo.writeDisable();
  }

  void startRead()
  {
    fifo.readReset();
  }

  void inline readFrame(unsigned char *frame, const int xres, const int yres, const int bytes)
  {
    startRead();
    int i = 0;
    for(int y = 0; y < yres; y++)
      for(int x = 0; x < xres; x++)
        for(int b = 0; b < bytes; b++)        
          frame[i++] = fifo.readByte();
  }

  void inline readPixels(unsigned char *pixels, const int count, const int bytes)
  {
    for(int i = 0; i < count * bytes; i++)
      pixels[i] = fifo.readByte();
  }
  
  void inline skipPixels(const int count, const int bytes)
  {
    fifo.skipBytes(count * bytes);
  }
  
  void inline readFrameOnlySecondByte(unsigned char *frame, const int xres, const int yres)
  {
    fifo.readReset();
    int i = 0;
    for(int y = 0; y < yres; y++)
      for(int x = 0; x < xres; x++)
      {
          fifo.skipByte();
          frame[i++] = fifo.readByte();
      }       
  }
  
  void writeRegisters(const unsigned char data[][2])
  {
    for(int i = 0; data[i][0] != 0xff; i++)
      i2c.writeRegister(ADDR, data[i][0], data[i][1]);
  }

  void saturation(int s)  //-2 to 2
  {
    //color matrix values
    i2c.writeRegister(ADDR, 0x4f, 0x80 + 0x20 * s);
    i2c.writeRegister(ADDR, 0x50, 0x80 + 0x20 * s);
    i2c.writeRegister(ADDR, 0x51, 0x00);
    i2c.writeRegister(ADDR, 0x52, 0x22 + (0x11 * s) / 2);
    i2c.writeRegister(ADDR, 0x53, 0x5e + (0x2f * s) / 2);
    i2c.writeRegister(ADDR, 0x54, 0x80 + 0x20 * s);
    i2c.writeRegister(ADDR, 0x58, 0x9e);  //matrix signs
  }

  void frameControl(int hStart, int hStop, int vStart, int vStop)
  {
    i2c.writeRegister(ADDR, REG_HSTART, hStart >> 3);
    i2c.writeRegister(ADDR, REG_HSTOP,  hStop >> 3);
    i2c.writeRegister(ADDR, REG_HREF, ((hStop & 0b111) << 3) | (hStart & 0b111));

    i2c.writeRegister(ADDR, REG_VSTART, vStart >> 2);
    i2c.writeRegister(ADDR, REG_VSTOP, vStop >> 2);
    i2c.writeRegister(ADDR, REG_VREF, ((vStop & 0b11) << 2) | (vStart & 0b11));
  }

  void QVGA()
  {
    //320x240 60fps(1/2)
    i2c.writeRegister(ADDR, REG_COM3, 0x04);  //DCW enable
    i2c.writeRegister(ADDR, REG_COM14, 0x19); //pixel clock divided by 4, manual scaling enable, DCW and PCLK controlled by register
    i2c.writeRegister(ADDR, REG_SCALING_XSC, 0x3a);
    i2c.writeRegister(ADDR, REG_SCALING_YSC, 0x35);
    i2c.writeRegister(ADDR, REG_SCALING_DCWCTR, 0x11); //downsample by 2
    i2c.writeRegister(ADDR, REG_SCALING_PCLK_DIV, 0xf1); //pixel clock divided by 2
    i2c.writeRegister(ADDR, REG_SCALING_PCLK_DELAY, 0x02);
  }
  
  void QQVGA()
  {
    //160x120 (1/4)
    //i2c.writeRegister(ADDR, REG_CLKRC, 0x01);
    i2c.writeRegister(ADDR, REG_COM3, 0x04);  //DCW enable
    i2c.writeRegister(ADDR, REG_COM14, 0x1a); //pixel clock divided by 4, manual scaling enable, DCW and PCLK controlled by register
    i2c.writeRegister(ADDR, REG_SCALING_XSC, 0x3a);
    i2c.writeRegister(ADDR, REG_SCALING_YSC, 0x35);
    i2c.writeRegister(ADDR, REG_SCALING_DCWCTR, 0x22); //downsample by 4
    i2c.writeRegister(ADDR, REG_SCALING_PCLK_DIV, 0xf2); //pixel clock divided by 4
    i2c.writeRegister(ADDR, REG_SCALING_PCLK_DELAY, 0x02);
  }

  void QQQVGA()
  {
    //80x60 (1/8)

    i2c.writeRegister(ADDR, REG_COM3, 0x04);  //DCW enable
    i2c.writeRegister(ADDR, REG_COM14, 0x1b); //pixel clock divided by 4, manual scaling enable, DCW and PCLK controlled by register
    i2c.writeRegister(ADDR, REG_SCALING_XSC, 0x3a);
    i2c.writeRegister(ADDR, REG_SCALING_YSC, 0x35);
    i2c.writeRegister(ADDR, REG_SCALING_DCWCTR, 0x33); //downsample by 8
    i2c.writeRegister(ADDR, REG_SCALING_PCLK_DIV, 0xf3); //pixel clock divided by 8
    i2c.writeRegister(ADDR, REG_SCALING_PCLK_DELAY, 0x2);
  }

  void QVGARGB565()
  {
    
    i2c.writeRegister(ADDR, REG_COM7, 0b10000000);  //all registers default
        
    i2c.writeRegister(ADDR, REG_CLKRC, 0b10000000); //double clock
    i2c.writeRegister(ADDR, REG_COM11, 0b1000 | 0b10); //enable auto 50/60Hz detect + exposure timing can be less...
    i2c.writeRegister(ADDR, REG_TSLB, 0b100); //sequence UYVY

    i2c.writeRegister(ADDR, REG_COM7, 0b100); //RGB
    i2c.writeRegister(ADDR, REG_COM15, 0b11000000 | 0b010000); //RGB565

    frameControl(196, 52, 8, 488); //no clue why horizontal needs such strange values, vertical works ok
    
    i2c.writeRegister(ADDR, REG_MVFP, 0x2b);  //mirror flip

    QVGA();
    
    i2c.writeRegister(ADDR, 0xb0, 0x84);// no clue what this is but it's most important for colors
    saturation(0);
    i2c.writeRegister(ADDR, 0x13, 0xe7); //AWB on
    i2c.writeRegister(ADDR, 0x6f, 0x9f); // Simple AWB
  }
  
  void QQVGARGB565()
  {
    
    i2c.writeRegister(ADDR, REG_COM7, 0b10000000);  //all registers default
        
    i2c.writeRegister(ADDR, REG_CLKRC, 0b10000000); //double clock
    i2c.writeRegister(ADDR, REG_COM11, 0b1000 | 0b10); //enable auto 50/60Hz detect + exposure timing can be less...
    i2c.writeRegister(ADDR, REG_TSLB, 0b100); //sequence UYVY

    i2c.writeRegister(ADDR, REG_COM7, 0b100); //RGB
    i2c.writeRegister(ADDR, REG_COM15, 0b11000000 | 0b010000); //RGB565

    frameControl(196, 52, 8, 488); //no clue why horizontal needs such strange values, vertical works ok
    
    i2c.writeRegister(ADDR, REG_MVFP, 0x2b);  //mirror flip

    QQVGA();
    
    i2c.writeRegister(ADDR, 0xb0, 0x84);// no clue what this is but it's most important for colors
    saturation(0);
    i2c.writeRegister(ADDR, 0x13, 0xe7); //AWB on
    i2c.writeRegister(ADDR, 0x6f, 0x9f); // Simple AWB
  }

  void QQQVGARGB565()
  {
    //still buggy
    i2c.writeRegister(ADDR, REG_COM7, 0b10000000);  //all registers default
        
    i2c.writeRegister(ADDR, REG_CLKRC, 0b10000000); //double clock
    i2c.writeRegister(ADDR, REG_COM11, 0b1000 | 0b10); //enable auto 50/60Hz detect + exposure timing can be less...
    i2c.writeRegister(ADDR, REG_TSLB, 0b100); //sequence UYVY

    i2c.writeRegister(ADDR, REG_COM7, 0b100); //RGB
    i2c.writeRegister(ADDR, REG_COM15, 0b11000000 | 0b010000); //RGB565

    frameControl(196, 52, 8, 488); //no clue why horizontal needs such strange values, vertical works ok
    
    i2c.writeRegister(ADDR, REG_MVFP, 0x2b);  //mirror flip

    QQQVGA();
    
    i2c.writeRegister(ADDR, 0xb0, 0x84);// no clue what this is but it's most important for colors
    saturation(0);
    i2c.writeRegister(ADDR, 0x13, 0xe7); //AWB on
    i2c.writeRegister(ADDR, 0x6f, 0x9f); // Simple AWB
  }

  void QQVGAYUV()
  {
    i2c.writeRegister(ADDR, REG_COM7, 0b10000000);  //all registers default
        
    i2c.writeRegister(ADDR, REG_CLKRC, 0b10000000); //double clock
    i2c.writeRegister(ADDR, REG_COM11, 0b1000 | 0b10); //enable auto 50/60Hz detect + exposure timing can be less...
    i2c.writeRegister(ADDR, REG_TSLB, 0b100); //sequence UYVY

    frameControl(196, 52, 8, 488); //no clue why horizontal needs such strange values, vertical works ok
    
    //i2c.writeRegister(ADDR, REG_COM10, 0x02); //VSYNC negative
    i2c.writeRegister(ADDR, REG_MVFP, 0x2b);  //mirror flip

    QQVGA();
    
    i2c.writeRegister(ADDR, 0xb0, 0x84);// no clue what this is but it's most important for colors
    i2c.writeRegister(ADDR, 0x13, 0xe7); //AWB on
    i2c.writeRegister(ADDR, 0x6f, 0x9f); // Simple AWB
  }
};

