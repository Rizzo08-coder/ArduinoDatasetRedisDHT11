#include <Redis.h>
// this sketch will build for the ESP8266 or ESP32 platform
#ifdef HAL_ESP32_HAL_H_ // ESP32
#include <WiFiClient.h>
#include <WiFi.h>
#else
#ifdef CORE_ESP8266_FEATURES_H // ESP8266
#include <ESP8266WiFi.h>
#endif
#endif
#include "DHT.h"  
#include "time.h" 

#define WIFI_SSID "INTRED-46132"
#define WIFI_PASSWORD "7D2BAC55109964D1AF4A"

#define REDIS_ADDR "192.168.178.25"
#define REDIS_PORT 6379
#define REDIS_PASSWORD "admin"

#define DHTPIN 13
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
WiFiClient redisConn;
Redis redis(redisConn);

//per ottenere ora attuale collegandomi a server NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 7200;
char currentTime[11];

void setup()
{
    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to the WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    
    if (!redisConn.connect(REDIS_ADDR, REDIS_PORT))
    {
        Serial.println("Failed to connect to the Redis server!");
        return;
    }

    auto connRet = redis.authenticate(REDIS_PASSWORD);
    if (connRet == RedisSuccess)
    {
        Serial.println("Connected to the Redis server!");
    }
    else
    {
        Serial.printf("Failed to authenticate to the Redis server! Errno: %d\n", (int)connRet);
        return;
    }
    dht.begin();

    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); //configuro ora attuale con parametri inseriti
  
}

void loop()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return ;
  }
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  char timeMinutes[3];
  strftime(timeMinutes,3, "%M", &timeinfo);
  char timeSeconds[3];
  strftime(timeSeconds,3, "%S", &timeinfo);
  char separator[] = ":" ;
  strcat(currentTime, timeHour);
  strcat(currentTime, separator);
  strcat(currentTime, timeMinutes);
  strcat(currentTime, separator);
  strcat(currentTime, timeSeconds);  //visualizzazione orario H:M:S

  
  char themsg[10];
  sprintf(themsg,"%f",float(dht.readTemperature()));
  //Serial.print(themsg);
  Serial.println();
  char keyTemp[10];
  sprintf(keyTemp,"roomTemp.%s", currentTime);
  if(redis.set(keyTemp, themsg)){
    Serial.println("key roomTemp load ok!");
    Serial.print(keyTemp);
    Serial.print(": "); 
    Serial.println(redis.get(keyTemp));
  }
  else{
    Serial.println("key roomTemp load falue");
  }

  char humsg[10];
  sprintf(humsg, "%d", int(dht.readHumidity()));
  char keyHum[10];
  sprintf(keyHum, "roomHum.%s", currentTime);
  if(redis.set(keyHum, humsg)){
    Serial.println("key roomHum load ok!");
    Serial.print(keyHum);
    Serial.print(": "); 
    Serial.println(redis.get(keyHum));
  }
  else{
    Serial.println("key roomHum load falue");
  }

  memset(currentTime, 0, sizeof(currentTime));
  delay(5000);
}
