//simple example 

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>

//comment this out if you don't want to use MQTT
#define USE_MQTT
//comment this out if you don't want to use a TFT display
#define USE_TFT

//Wifi credentials
#define ssid1        "YOUR_WIFI_SSID"
#define password1    "YOUR_WIFI_PASSWORD"
//#define ssid2        ""
//#define password2    ""

//mqtt credentials
#ifdef USE_MQTT
  #define MQTT_SERVER      "192.168.0.10"
  #define MQTT_SERVERPORT  1883
  #define MQTT_USERNAME    "YOUR_MQTT_USERNAME"
  #define MQTT_KEY         "YOUR_MQTT_PASSWORD"
  #define MQTT_TOPIC       "home/camera1"
  #include "MQTTStuff.h"
#endif

#include "I2C.h"
#include "FifoCamera.h"
#include "BMP.h"

#include "Pins.h"

I2C<SIOD, SIOC> i2c;
FifoCamera<I2C<SIOD, SIOC>, RRST, WRST, RCK, WR, D0, D1, D2, D3, D4, D5, D6, D7> camera(i2c);

#ifdef USE_TFT
#include <SPI.h>
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);
#endif

const int XRES = 160;
const int YRES = 120;

const int BYTES_PER_PIXEL = 2;
unsigned char bmpHeader[BMP::headerSize];
//unsigned char frameLine[XRES * BYTES_PER_PIXEL];
unsigned char frame[XRES  * YRES  * BYTES_PER_PIXEL];   
#ifdef USE_TFT
#include "Display.h"
#endif

WiFiMulti wifiMulti;

WiFiServer server(80);

void setup() 
{
  Serial.begin(115200);
  wifiMulti.addAP(ssid1, password1);
  //wifiMulti.addAP(ssid2, password2);
  Serial.println("Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
  }
  
  int retry = 3;
  Serial.println(WiFi.localIP());
  
  #ifdef USE_MQTT
  MQTTConnect();
  while(!mqttcamera.publish(WiFi.localIP().toString().c_str()) && retry)   
  {
    retry--;
    delay(1000);
  }
  mqtt.disconnect();
  #endif
  
  i2c.init();
  camera.init();

  BMP::construct16BitHeader(bmpHeader, XRES, YRES);
  camera.QQVGARGB565();
  
  pinMode(VSYNC, INPUT);
  #ifdef USE_TFT
  Serial.println("Reset display.");
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(0);
  testTFT();
  #endif
  Serial.println("Server start");
  server.begin();
}

void serve()
{
  WiFiClient client = server.available();
  if (client) 
  {
    //Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        //Serial.write(c);
        if (c == '\n') 
        {
          if (currentLine.length() == 0) 
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(
              "<style>body{margin: 0}\nimg{height: 100%; width: auto}</style>"
              "<img id='a' src='/camera' onload='this.style.display=\"initial\"; var b = document.getElementById(\"b\"); b.style.display=\"none\"; b.src=\"camera?\"+Date.now(); '>"
              "<img id='b' style='display: none' src='/camera' onload='this.style.display=\"initial\"; var a = document.getElementById(\"a\"); a.style.display=\"none\"; a.src=\"camera?\"+Date.now(); '>");
            client.println();
            break;
          } 
          else 
          {
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {
          currentLine += c;
        }
        
        if(currentLine.endsWith("GET /camera"))
        {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:image/bmp");
            client.println();
            
            for(int i = 0; i < BMP::headerSize; i++)
               client.write(bmpHeader[i]);
            for(int i = 0; i < XRES * YRES * BYTES_PER_PIXEL; i++)
               client.write(frame[i]);
               
            /*camera.startRead();
            for(int y = 0; y < YRES; y++)
            {
              camera.readPixels(frameLine, XRES, BYTES_PER_PIXEL);
              for(int x = 0; x < XRES * BYTES_PER_PIXEL; x ++)
                client.write(frameLine[x]);
            }    */
        }
      }
    }
    // close the connection:
    client.stop();
    //Serial.println("Client Disconnected.");
  }  
}

void readFrame()
{
  while(!digitalRead(VSYNC));
  while(digitalRead(VSYNC));
  camera.stopCapture();
  camera.prepareCapture();
  while(!digitalRead(VSYNC));
  camera.startCapture();
  camera.readFrame(frame, XRES, YRES, BYTES_PER_PIXEL);
}

void loop() 
{
  readFrame();
  #ifdef USE_TFT
  displayRGB565(frame, XRES, YRES);
  #endif
  serve();
}
