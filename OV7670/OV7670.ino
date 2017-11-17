#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include "I2C.h"
#include "FifoCamera.h"

const int VSYNC = 32;
const int SIOD = 21; //SDA
const int SIOC = 22; //SCL

const int RRST = 17;
const int WRST = 16; 
const int RCK = 4;
const int WR = 0;

const int D0 = 13;
const int D1 = 12;
const int D2 = 14;
const int D3 = 27;
const int D4 = 26;
const int D5 = 25;
const int D6 = 35;
const int D7 = 34;

const int TFT_CS = 2;
const int TFT_RST = 0;
const int TFT_DC = 15;
  
I2C<SIOD, SIOC> i2c;
FifoCamera<I2C<SIOD, SIOC>, RRST, WRST, RCK, WR, D0, D1, D2, D3, D4, D5, D6, D7> camera(i2c);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

TaskHandle_t cameraTask;

const int XRES = 160;
const int YRES = 120;
const int BYTES_PER_PIXEL = 2;
const int frameSize = XRES * YRES * BYTES_PER_PIXEL;
unsigned char frame[frameSize];

void cameraTaskProc(void * params)
{
  while(true)
  {
    Serial.print('f');
    Serial.print('d');
    delay(1);
  }
}

void setup() 
{
  Serial.begin(115200);
  Serial.println("Initialization...");
  i2c.init();
  camera.init();
  //camera.QQVGARGB565();
  camera.QQVGARGB();
  //camera.RGBRaw();
  //camera.testImage();
  pinMode(VSYNC, INPUT);
  Serial.println("start");
  //attachInterrupt(digitalPinToInterrupt(VSYNC), vsyncInt, RISING);
  //xTaskCreatePinnedToCore(cameraTaskProc, "cam", 10000, NULL, 1, &cameraTask, 0);
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(0);
}

void loop() 
{
  while(!digitalRead(VSYNC));
  while(digitalRead(VSYNC));
  camera.prepareCapture();
  camera.startCapture();
  while(!digitalRead(VSYNC));
  camera.stopCapture();

  camera.readFrame(frame, XRES, YRES, BYTES_PER_PIXEL);
  tft.setAddrWindow(0,0,119,159);
  int i = 0;
  for(int x = 0; x < 160; x++)
    for(int y = 0; y < 120; y++)
    {
      i = (y * 160 + x) << 1;
      /*
      unsigned short r = frame[i + 1] >> 3;
      unsigned short g = (frame[i] >> 5) | ((frame[i + 1] & 0b111) << 3);
      unsigned short b = frame[i] & 0b11111;
      unsigned short c = (r << 11) | (g << 5) | (b << 0);*/
      //tft.pushColor(c);
      tft.pushColor(frame[i] | (frame[i + 1] << 8));
      //tft.pushColor(frame[i + 1] >> 3);
    }
/*  Serial.print("frame:");
  long size = XRES * YRES;
  Serial.write((unsigned char*)&size, 4);
  Serial.write((unsigned char*)frame, XRES * YRES);*/
  /*
  int i = 0;
  Serial.println("var frame=[");
  for(int y = 0; y < YRES; y+=1)
  {
    i = y * XRES;
    for(int x = 0; x < XRES; x+=1)
    {
      //Serial.print(frame[i + x], HEX);
      Serial.print(frame[i + x]);
      Serial.print(',');
    }
    Serial.println();
  }
  Serial.println("];");*/
}
