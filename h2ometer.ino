#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "h2ometer.h"

void callback(char* topic, byte* payload, unsigned int length);

Adafruit_HMC5883_Unified mag;

WiFiClient wifiClient; //for MQTT
PubSubClient client(C.mqtt_server, 1883, callback, wifiClient);

//WiFiClient wifiClientTS; //for ThingSpeak

void callback(char* topic, byte* payload, unsigned int length) {
  char message_buff[length+1]; //+1 for terminating null
  char* tmp;
  char* gcmd;
  char* topicname = strdup(topic); //local copy since strtok is going to modify
  if (topicname == 0 ) { return; }

  snprintf(message_buff,length+1,"%s",payload); //make payload a null term char array

//  Serial.print("Message arrived: topic: ");
//  Serial.print(topicname);
//  Serial.print(" message: ");
//  Serial.println(message_buff);
  
  tmp = strtok(topicname,"/");
  if (strcmp(tmp, C.mqtt_topicbase) == 0) {
      tmp = strtok(NULL,"/");
      if (strcmp(tmp, "dev/announce")  == 0) {
        tmp = strtok(NULL,"/");
        if (strcmp(gcmd=strtok(NULL,"/"),"last") == 0) {
//          Serial.print("  Message in a _last_ gauge topic - gauge# ");
//          Serial.print(tmp);
//          Serial.print(" set to: ");
//          Serial.println(message_buff);
        } else if (strcmp(gcmd, "max") == 0) {
//          Serial.print("  Message in a _max_ gauge topic - gauge# ");
//          Serial.print(atoi(tmp));
//          Serial.print(" set to: ");
//          Serial.println(atoi(message_buff));
        }
      }
  }
  free(topicname);
}

void setup() {

  Serial.begin(9600);
  Serial.println("Water Meter MQTT Sensor");

  Wire.begin(PIN_I2C_SDA,PIN_I2C_SCL);
  delay(10);

  Serial.print("ssid:");Serial.println(C.wifi_ssid);
  WiFi.begin(C.wifi_ssid, C.wifi_pwd);

  Serial.print("ESP-01 MAC: ");
  uint8_t mac[6];
  WiFi.macAddress(mac);
  for (int i=0; i<sizeof(mac); ++i) {
    sprintf(MAC_char,"%s%02x:",MAC_char,mac[i]);
  }
  MAC_char[strlen(MAC_char)-1]='\0';
  Serial.println(MAC_char);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Generate client name based on MAC address and last 8 bits of microsecond counter
//  sprintf(MQTTClientName,"%-17s-%02x", MAC_char, micros() & 0xff,16);
//  sprintf(MQTTClientName,"%-17s-%s", MAC_char, "xy"); //For debugging set -xx so that I'm not chasing the clientname
  sprintf(MQTTClientName,"%-17s", MAC_char);
  
  Serial.print("Connecting to ");
  Serial.print(C.mqtt_server);
  Serial.print(" as ");
  Serial.println(MQTTClientName);

  if (client.connect(MQTTClientName)) {
    Serial.println("Connected to MQTT broker");

    snprintf(MQTTTopicConfig,50,"%s/%s/config",C.mqtt_topicbase,MQTTClientName);
    MQTTTopicConfig[50]='\0';
    if (client.subscribe(MQTTTopicConfig,0)) {
      Serial.print("Subscribe to configuration topic ok: ");
      Serial.println(MQTTTopicConfig);
    } else {
      Serial.print("Subscribe to configuration topic failed: ");
      Serial.println(MQTTTopicConfig);
    }

    snprintf(MQTTTopicAnnounce,50,"%s/dev/announce",C.mqtt_topicbase);
    MQTTTopicAnnounce[49]='\0';
    if (client.publish(MQTTTopicAnnounce, MQTTClientName)) {
      Serial.println("Publish to dev/announce topic ok");
    } else {
      Serial.println("Publish to dev/announce topic failed");
    }
  } else {
    Serial.println("MQTT connect failed");
//    Serial.println("Will reset and try again...");
//    abort();
  }

//  snprintf(MQTTTopicHumd,50,"%s/%s/h2ocnt",C.mqtt_topicbase,MQTTClientName);
//  MQTTTopicHumd[49]='\0';
//  Serial.print("MQTTTopicHumd:");  Serial.println(MQTTTopicHumd);
//  snprintf(MQTTTopicTemp,50,"%s/%s/temp",C.mqtt_topicbase,MQTTClientName);
//  MQTTTopicTemp[49]='\0';
//  Serial.print("MQTTTopicTemp:");  Serial.println(MQTTTopicTemp);
//  snprintf(MQTTTopicDewp,50,"%s/%s/dewp",C.mqtt_topicbase,MQTTClientName);
//  MQTTTopicDewp[49]='\0';
//  Serial.print("MQTTTopicDewp:");  Serial.println(MQTTTopicDewp);
//  snprintf(MQTTTopicPPres,50,"%s/%s/ppres",C.mqtt_topicbase,MQTTClientName);
//  MQTTTopicPPres[49]='\0';
//  Serial.print("MQTTTopicPPres:");  Serial.println(MQTTTopicPPres);
//  snprintf(MQTTTopicSigRH,50,"%s/%s/sigrh",C.mqtt_topicbase,MQTTClientName);
//  MQTTTopicSigRH[49]='\0';
//  Serial.print("MQTTTopicSigRH:");  Serial.println(MQTTTopicSigRH);
//  snprintf(MQTTTopicSigT,50,"%s/%s/sigt",C.mqtt_topicbase,MQTTClientName);
//  MQTTTopicSigT[49]='\0';
//  Serial.print("MQTTTopicSigT:");  Serial.println(MQTTTopicSigT);

}

void loop() {

  client.loop();

//sendThingspeak (temp, humd, ppres, sigRH, sigT, dewp );

  Serial.print("Time:");
  Serial.println(millis());
  

  delay(C.sensor_period);
}
