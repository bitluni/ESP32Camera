#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"


WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);
Adafruit_MQTT_Publish mqttcamera = Adafruit_MQTT_Publish(&mqtt, MQTT_TOPIC);
Adafruit_MQTT_Subscribe mqttcameraonoff = Adafruit_MQTT_Subscribe(&mqtt, "homeCamera/onoff");

bool MQTTConnect() {
  int8_t ret;

  if (mqtt.connected()) {
    return true;
  }
  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection");
       mqtt.disconnect();
       delay(1000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         Serial.println("MQTT connection failed");
         return false;
       }
  }
  Serial.println("MQTT connected");
  return true;
}

void MQTTLoop()
{
    //MQTT_connect();
/*  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &mqttcameraonoff) {
      Serial.print(F("Got: "));
      Serial.println((char *)mqttcameraonoff.lastread);
    }
  }
*/
  // Now we can publish stuff!
  //max 64kb
  /*if (!mqttcamera.publish(bmp, 66)) 
  {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }*/
}

