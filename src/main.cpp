//define Libraries
#include<Arduino.h>
//temperature libraries
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
//Wifi libraries
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <LabeebIoT.h>

//--------------------------------------------------------

//define GPIOs
#define temperatureSensorPin 23   
#define humiditySensorPin 22 
#define piezoSensorPin 36
#define ledWifi 32
#define ledServer 27
//--------------------------------------------------------

//define User credantials
const char* wifiSsid     = "";
const char* wifiPassword = "";
const char* entreprise   = "";
const char* mqttServer   = "mea.labeeb-iot.com";
const char* clientID     = "";
const char* templateID   = "";
const char* templatePassword = "";
//--------------------------------------------------------

//define Variables
int periode = 10000;
float temperature;
float humidity;
float vibration;
char temperatureArray[10];
char humidityArray[10];
char vibrationArray[10];
DHT_Unified temperatureSensorDHT22(temperatureSensorPin, DHT22);
DHT_Unified humiditySensorDHT21(humiditySensorPin, DHT21);
bool wifiFlag = true;
bool serverFlag = true;
WiFiUDP ntpUDP;
WiFiClient espClient;
LabeebIoT labeeb (espClient,mqttServer,1883,entreprise);
NTPClient timeClient(ntpUDP, "africa.pool.ntp.org", 3600, 60000);
//--------------------------------------------------------

//define Functions
void readTemperature()
{
  sensors_event_t event;  
  temperatureSensorDHT22.temperature().getEvent(&event);
  if (isnan(event.temperature)) 
    Serial.println("Error reading temperature!");
  else 
  {
    temperature=event.temperature;
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
    snprintf(temperatureArray, sizeof(temperatureArray), "%f", temperature);
  }
}

void readTemperature2()
{
  sensors_event_t event; 
  humiditySensorDHT21.temperature().getEvent(&event);
  if (isnan(event.relative_humidity)) 
    Serial.println("Error reading humidity!");
  else 
  {
    temperature=event.temperature;
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" *C");
    snprintf(temperatureArray, sizeof(temperatureArray), "%f", temperature);
  }
}

void readHumidity()
{
  sensors_event_t event;  
  humiditySensorDHT21.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) 
    Serial.println("Error reading humidity!");
  else 
  {
    humidity=event.relative_humidity;
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    snprintf(humidityArray, sizeof(humidityArray), "%f", humidity);
  }
}

void readVibration()
{
  vibration=(analogRead(piezoSensorPin)*3.3)/3999;
//sensitivity : 50 mV/g
  vibration=(vibration/50)*1000; 
  Serial.print("Vibration: ");
  Serial.print(vibration);
  Serial.println(" kg");
  snprintf(vibrationArray, sizeof(vibrationArray), "%f", vibration);
}

void blinkConnectWifi(bool stat)
{
  if(stat==true)
    digitalWrite(ledWifi,HIGH);
  else
    digitalWrite(ledWifi,LOW);
}

void connectWifi()
{
  Serial.print("Connecting to ");
  Serial.println(wifiSsid);
  WiFi.begin(wifiSsid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) 
  {   
        blinkConnectWifi(wifiFlag);
        delay(500);
        Serial.print(".");
        wifiFlag = not(wifiFlag);
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void blinkConnectServer(bool stat)
{
  if(stat==true)
    digitalWrite(ledServer,HIGH);
  else
    digitalWrite(ledServer,LOW);
}

void reconnectToLabeeb()
 {
  while (!labeeb.connected()) 
  {
    if (labeeb.connectLabeeb(clientID,templateID,templatePassword))
    {
      Serial.println("Labeeb Server connected");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(labeeb.state());
      Serial.println("reconnecting");
      blinkConnectServer(serverFlag);
      serverFlag = not(serverFlag);
      delay(1000);
    }
  }
}

void doEveryPeriod()
{
  timeClient.update();
  digitalWrite(ledWifi,HIGH);
  digitalWrite(ledServer,HIGH);
  Serial.println(timeClient.getFormattedTime());
  delay(periode);
}
//--------------------------------------------------------


void setup()
{
  Serial.begin(9600); 
  timeClient.begin();
  temperatureSensorDHT22.begin();
  humiditySensorDHT21.begin();
  pinMode(ledWifi,OUTPUT);
  pinMode(ledServer,OUTPUT);
  pinMode(piezoSensorPin,INPUT);
  connectWifi();
  reconnectToLabeeb();
  Serial.println("DHT22 initialized");
  Serial.println("DHT21 initialized");
  delay(500);
}

void loop() 
{
  labeeb.loop();
  doEveryPeriod();
  readHumidity();
   labeeb.publishLabeebValue("ESP32","humidity monitoring","humidity",humidityArray,(char*)timeClient.getEpochTime(),"36.809285","10.170593","1");
  delay(500);
  readTemperature();
  labeeb.publishLabeebValue("ESP32","temperature monitoring","temperature",temperatureArray,(char*)timeClient.getEpochTime(),"36.809285","10.170593","1");
  delay(500);
  readVibration();
  labeeb.publishLabeebValue("ESP32","Vibration monitoring","Vibration",vibrationArray,(char*)timeClient.getEpochTime(),"36.809285","10.170593","1");
  delay(500);
  
 
 }

 
