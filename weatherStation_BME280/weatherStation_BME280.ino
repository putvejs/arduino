
#include <Arduino.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "arduino_secrets.h"

ADC_MODE(ADC_VCC);
int Batt;

#define BME_SCK 14
#define BME_MISO 12
#define BME_MOSI 13
#define BME_CS 0



#define SEALEVELPRESSURE_HPA (1013.25)
float temperature, humidity, pressure, alt;
boolean publishStatus, status;
String ssidName;
Adafruit_BME280 bme;

//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

//wifi definition
char ssid[]     = SECRET_SSID;
char password[] = SECRET_PASS;

IPAddress staticIP(192,168,8,4);
IPAddress gateway(192,168,8,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(192,168,8,130);

WiFiClient espClient;

//mqtt definition

String MQTT_TOPIC_STATE = CLIENT_ID+"/"+DEVICE_ID+"/status";

//#define MQTT_TOPIC_STATE 
#define MQTT_PUBLISH_DELAY 60000


const char *MQTT_SERVER = MQTT_SERVER_ID;
const char *MQTT_USER = MQTT_USER_ID ; // NULL for no authentication
const char *MQTT_PASSWORD = MQTT_PASSWORD_ID ; // NULL for no authentication

PubSubClient mqttClient(espClient);

void setup(void) {
  Serial.begin(115200);
  Serial.setTimeout(2000);

  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  
   //Wifi Setup
    //WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    connectWifi();


//mqtt setup
    mqttClient.setServer(MQTT_SERVER, 1883);

    if (!mqttClient.connected()) {
      mqttReconnect();
    }
    mqttClient.loop();
  

  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure()/ 100.0F;
  alt = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Batt = ESP.getVcc();


    mqttClient.publish(String(MQTT_TOPIC_STATE).c_str(), "BME readings done",true);
    publishStatus =  mqttClient.publish("pagrabs/bme280/temp", String(temperature).c_str(),true);
    publishStatus =  mqttClient.publish("pagrabs/bme280/hum", String(humidity).c_str(),true);
    publishStatus =  mqttClient.publish("pagrabs/bme280/sp", ,true);
    publishStatus =  mqttClient.publish("pagrabs/bme280/alt", String(alt).c_str(),true);
    publishStatus =  mqttClient.publish("pagrabs/bme280/vv", String(Batt).c_str(),true);
    delay(2000);   
     
    if (publishStatus) {
           Serial.println("Data publishing was successifull");
           mqttClient.publish(MQTT_TOPIC_STATE, "BME readings published to MQTT",true);
           delay(2000);   
    }

  
    //ESP.deepSleep(18000);
   ESP.deepSleep(600e6);
   //delay(1800000);
   

}

void loop(void) {

 
      
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");

      // Once connected, publish an announcement...

      } else {
        Serial.print("failed, rc=");
        Serial.print(mqttClient.state());
        Serial.println(" try again in 5 seconds");
      }
  }
}

 void connectWifiDiagnostics() {
        Serial.println("Connected to " + String(ssidName));
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.println(WiFi.macAddress());
        Serial.println(WiFi.subnetMask());
        Serial.println(WiFi.gatewayIP());
        Serial.println(WiFi.dnsIP());
        Serial.println(WiFi.status());
        WiFi.printDiag(Serial);
 }

void connectWifi() {

  while (WiFi.status() != WL_CONNECTED) {

  if (WiFi.status() != WL_CONNECTED){

    WiFi.disconnect();
  }
        
      WiFi.begin(ssid, password);
      ssidName = ssid;
      delay(5000);

     
      if (WiFi.status () == WL_CONNECTED) {
        
      connectWifiDiagnostics();
      
          WiFiClient client;
          if (!client.connect(MQTT_SERVER, 1883)) {
            Serial.println("connection failed");
            WiFi.config(staticIP,gateway, subnet, dns);
            connectWifi();  
            connectWifiDiagnostics();
            return;

        }
          else {
            Serial.println("Internet connection working");
          }
      
      
  }
    
}
}
