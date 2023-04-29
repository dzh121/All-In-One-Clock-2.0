/* 
  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  String time_str;
  int dayMonth;
  int dayWeek;
  int hours;
  int humidity;
  int minutes;+++
  int monthNow;
  int pressure;
  int temp;
  int yearNow;
  bool override_send;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/

#include "thingProperties.h"
#include <Wire.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include "TimeLib.h"
#include <WiFiClient.h>


// Set up WiFi network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

const char* host = "api.weatherapi.com";
const int port = 80;

// Your API key
const char* apiKey = "KEY";

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, host, port);

unsigned long previousMillis = 0;        // will store last time LED was updated
// constants won't change:
const long interval = 300000; //5 min
// const long interval = 5000; //5 sec

unsigned long previousMillis1 = 0;        // will store last time LED was updated
// constants won't change:
const long interval1 = 30000; //30 sec

int minuteOld = 0;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  Wire.begin();
  delay(1000);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");
  //timeZoneOffset = ArduinoCloud.getTimeZoneOffset();
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();
  //5 min delay
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval || override_send) {
    previousMillis = currentMillis;
    if (client.connect(host, port)) {
      Serial.println("Connected to server");
      String url = "/v1/forecast.json?key=";
      url += apiKey;
      url += "&q=modiin,IL&days=3";
      
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n"); // remove the headers section
  
      while (!client.available()) {
        delay(10);
      }
      String response = client.readString();
      Serial.println("----");
      Serial.println(response);
      int firstOpeiningBracket = response.indexOf('{');
      String data = response.substring(firstOpeiningBracket);
      
      StaticJsonDocument<5000> doc;
      
      DeserializationError error = deserializeJson(doc, data);
  
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.f_str());
        return;
      }
  
      temp = doc["current"]["temp_c"];
      humidity = doc["current"]["humidity"];
      pressure = doc["current"]["pressure_mb"];
  
  
      String todayIconUrl = doc["current"]["condition"]["icon"];
      
      StaticJsonDocument<200> jsonOpenWeatherMapTHP;
  
      jsonOpenWeatherMapTHP["tw"] = temp;
      jsonOpenWeatherMapTHP["hw"] = humidity;
      jsonOpenWeatherMapTHP["pw"] = pressure;
      
      String jsonStringOpenWeatherMapTHP;
      serializeJson(jsonOpenWeatherMapTHP, jsonStringOpenWeatherMapTHP);
      Wire.beginTransmission(8);
      if(jsonStringOpenWeatherMapTHP.length() < 34){
        Wire.write(jsonStringOpenWeatherMapTHP.c_str(), jsonStringOpenWeatherMapTHP.length());
      }else{
        Serial.println("Too long max of 32 bytes");
      }
      Wire.endTransmission();
      
      StaticJsonDocument<200> jsonOpenWeatherMapTodayIcon;
      
      int start = todayIconUrl.lastIndexOf("/") + 1;
      int end = todayIconUrl.lastIndexOf(".");
      String numStr = todayIconUrl.substring(start, end);
      int num = numStr.toInt();
      
      String timeOfDayForImageToday = "day";
      if (todayIconUrl.indexOf("day") != -1) {
        timeOfDayForImageToday = "day";
      } else if (todayIconUrl.indexOf("night") != -1) {
        timeOfDayForImageToday = "night";
      }
      
      jsonOpenWeatherMapTodayIcon["int"] = num;
      jsonOpenWeatherMapTodayIcon["nort"] = timeOfDayForImageToday;
      
      
      String jsonStringOpenWeatherMapTodayIcon;
      serializeJson(jsonOpenWeatherMapTodayIcon, jsonStringOpenWeatherMapTodayIcon);
      Wire.beginTransmission(8);
      if(jsonStringOpenWeatherMapTodayIcon.length() < 34){
        Wire.write(jsonStringOpenWeatherMapTodayIcon.c_str(), jsonStringOpenWeatherMapTodayIcon.length());
      }else{
        Serial.println("Too long max of 32 bytes");
      }
      Wire.endTransmission();
      
      StaticJsonDocument<200> jsonOpenWeatherMapTodayFullInfo;
      
      int feels_like = doc["current"]["feelslike_c"];
      int uv = doc["current"]["uv"];
      float wind_speed = doc["current"]["wind_kph"];
      jsonOpenWeatherMapTodayFullInfo["flt"] = feels_like;
      jsonOpenWeatherMapTodayFullInfo["uv"] = uv;
      jsonOpenWeatherMapTodayFullInfo["wst"] = String(wind_speed,1);
      
      String jsonStringOpenWeatherMapFullInfo;
      serializeJson(jsonOpenWeatherMapTodayFullInfo, jsonStringOpenWeatherMapFullInfo);
      Wire.beginTransmission(8);
      if(jsonStringOpenWeatherMapFullInfo.length() < 34){
        Wire.write(jsonStringOpenWeatherMapFullInfo.c_str(), jsonStringOpenWeatherMapFullInfo.length());
      }else{
        Serial.println("Too long max of 32 bytes");
      }
      Wire.endTransmission();
      
      StaticJsonDocument<200> jsonOpenWeatherMapTodayForecast;
      
      int todayMaxTemp = doc["forecast"]["forecastday"][0]["day"]["maxtemp_c"];
      int todayMinTemp = doc["forecast"]["forecastday"][0]["day"]["mintemp_c"];
      int todayRain = doc["forecast"]["forecastday"][0]["day"]["daily_chance_of_rain"];
      String todayIcon = doc["forecast"]["forecastday"][0]["day"]["condition"]["icon"];
      
      start = todayIcon.lastIndexOf("/") + 1;
      end = todayIcon.lastIndexOf(".");
      numStr = todayIcon.substring(start, end);
      int iconNumberTodayForecast = numStr.toInt();
      
      jsonOpenWeatherMapTodayForecast["t1mx"] = todayMaxTemp;
      jsonOpenWeatherMapTodayForecast["t1mi"] = todayMinTemp;
      jsonOpenWeatherMapTodayForecast["t1r"] = todayRain;

      String jsonStringOpenWeatherMapTodayForecast;
      serializeJson(jsonOpenWeatherMapTodayForecast, jsonStringOpenWeatherMapTodayForecast);
      Wire.beginTransmission(8);
      if(jsonStringOpenWeatherMapTodayForecast.length() < 34){
        Wire.write(jsonStringOpenWeatherMapTodayForecast.c_str(), jsonStringOpenWeatherMapTodayForecast.length());
      }else{
        Serial.println("Too long max of 32 bytes");
      }
      Wire.endTransmission();
      
      StaticJsonDocument<200> jsonOpenWeatherMapTommorowForecast;
      
      int tommorowMaxTemp = doc["forecast"]["forecastday"][1]["day"]["maxtemp_c"];
      int tommorowMinTemp = doc["forecast"]["forecastday"][1]["day"]["mintemp_c"];
      int tommorowRain = doc["forecast"]["forecastday"][1]["day"]["daily_chance_of_rain"];
      String tommorowIcon = doc["forecast"]["forecastday"][1]["day"]["condition"]["icon"];
      
      start = tommorowIcon.lastIndexOf("/") + 1;
      end = tommorowIcon.lastIndexOf(".");
      numStr = tommorowIcon.substring(start, end);
      int iconNumberTommorowForecast = numStr.toInt();
      
      jsonOpenWeatherMapTommorowForecast["t2mx"] = tommorowMaxTemp;
      jsonOpenWeatherMapTommorowForecast["t2mi"] = tommorowMinTemp;
      jsonOpenWeatherMapTommorowForecast["t2r"] = tommorowRain;

      String jsonStringOpenWeatherMapTommorowForecast;
      serializeJson(jsonOpenWeatherMapTommorowForecast, jsonStringOpenWeatherMapTommorowForecast);
      Wire.beginTransmission(8);
      if(jsonStringOpenWeatherMapTommorowForecast.length() < 34){
        Wire.write(jsonStringOpenWeatherMapTommorowForecast.c_str(), jsonStringOpenWeatherMapTommorowForecast.length());
      }else{
        Serial.println("Too long max of 32 bytes");
      }
      Wire.endTransmission();
      
      StaticJsonDocument<200> jsonOpenWeatherMapTommorow1Forecast;
      
      int tommorow1MaxTemp = doc["forecast"]["forecastday"][2]["day"]["maxtemp_c"];
      int tommorow1MinTemp = doc["forecast"]["forecastday"][2]["day"]["mintemp_c"];
      int tommorow1Rain = doc["forecast"]["forecastday"][2]["day"]["daily_chance_of_rain"];
      String tommorow1Icon = doc["forecast"]["forecastday"][2]["day"]["condition"]["icon"];
      
      start = tommorow1Icon.lastIndexOf("/") + 1;
      end = tommorow1Icon.lastIndexOf(".");
      numStr = tommorow1Icon.substring(start, end);
      int iconNumberTommorow1Forecast = numStr.toInt();
      
      jsonOpenWeatherMapTommorow1Forecast["t3mx"] = tommorow1MaxTemp;
      jsonOpenWeatherMapTommorow1Forecast["t3mi"] = tommorow1MinTemp;
      jsonOpenWeatherMapTommorow1Forecast["t3r"] = tommorow1Rain;

      String jsonStringOpenWeatherMapTommorow1Forecast;
      serializeJson(jsonOpenWeatherMapTommorow1Forecast, jsonStringOpenWeatherMapTommorow1Forecast);
      Wire.beginTransmission(8);
      if(jsonStringOpenWeatherMapTommorow1Forecast.length() < 34){
        Wire.write(jsonStringOpenWeatherMapTommorow1Forecast.c_str(), jsonStringOpenWeatherMapTommorow1Forecast.length());
      }else{
        Serial.println("Too long max of 32 bytes");
      }
      Wire.endTransmission();
      
      StaticJsonDocument<200> jsonOpenWeatherMapForecastIcons;
      
      jsonOpenWeatherMapForecastIcons["t1c"] = iconNumberTodayForecast;
      jsonOpenWeatherMapForecastIcons["t2c"] = iconNumberTommorowForecast;
      jsonOpenWeatherMapForecastIcons["t3c"] = iconNumberTommorow1Forecast;

      String jsonStringOpenWeatherMapForecastIcons;
      serializeJson(jsonOpenWeatherMapForecastIcons, jsonStringOpenWeatherMapForecastIcons);
      Wire.beginTransmission(8);
      if(jsonStringOpenWeatherMapForecastIcons.length() < 34){
        Wire.write(jsonStringOpenWeatherMapForecastIcons.c_str(), jsonStringOpenWeatherMapForecastIcons.length());
      }else{
        Serial.println("Too long max of 32 bytes");
      }
      Wire.endTransmission();
      
      
      Serial.print("Temperature: ");
      Serial.println(temp);
      Serial.print("Humidity: ");
      Serial.println(humidity);
      Serial.print("Pressure: ");
      Serial.println(pressure);
      Serial.println(todayMaxTemp);
      Serial.print("Today's Min Temperature: ");
      Serial.println(todayMinTemp);
      Serial.print("Today's Icon URL: ");
      Serial.println(todayIconUrl);
      
      client.stop();
    } else {
      Serial.println("Connection failed");
    }
  }
  time_t utcCalc = ArduinoCloud.getLocalTime();
  minutes = minute(utcCalc);
  if (minutes != minuteOld) {
    previousMillis1 = currentMillis;
    time_t utcCalc = ArduinoCloud.getLocalTime();
    hours = hour(utcCalc);
    minutes = minute(utcCalc);
    monthNow = month(utcCalc);
    yearNow = year(utcCalc);
    dayMonth = day(utcCalc);
    dayWeek = weekday(utcCalc);
    
    StaticJsonDocument<200> jsonTimeHMM;
    jsonTimeHMM["ht"] = hours;
    jsonTimeHMM["mit"] = minutes;
    jsonTimeHMM["mot"] = monthNow;
    
    StaticJsonDocument<200> jsonTimeYDD;
    jsonTimeYDD["yt"] = yearNow;
    jsonTimeYDD["dmt"] = dayMonth;
    jsonTimeYDD["dwt"] = dayWeek;
    
    time_str = String(hours/10)+String(hours%10)+":"+String(minutes/10)+String(minutes%10);
    
    String jsonStringTimeHMM;
    serializeJson(jsonTimeHMM, jsonStringTimeHMM);
    String jsonStringTimeYDD;
    serializeJson(jsonTimeYDD, jsonStringTimeYDD);
    Wire.beginTransmission(8);
    if(jsonStringTimeHMM.length() < 34){
      Wire.write(jsonStringTimeHMM.c_str(), jsonStringTimeHMM.length());
    }else{
      Serial.println("Too long max of 32 bytes");
    }
    Wire.endTransmission();
    Wire.beginTransmission(8);
    if(jsonStringTimeYDD.length() < 34){
      Wire.write(jsonStringTimeYDD.c_str(), jsonStringTimeYDD.length());
    }else{
      Serial.println("Too long max of 32 bytes");
    }
    Wire.endTransmission();
    
    
    Serial.print(hours);
    Serial.print(":");
    Serial.println(minutes);
    Serial.print(dayMonth);
    Serial.print(".");
    Serial.print(monthNow);
    Serial.print(".");
    Serial.print(yearNow);
    Serial.print(".");
    Serial.println(dayWeek);
    
  }
  minuteOld = minutes;
}

/*
  Since DayWeek is READ_WRITE variable, onDayWeekChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onDayWeekChange()  {
  // Add your code here to act upon DayWeek change
}
/*
  Since Minutes is READ_WRITE variable, onMinutesChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMinutesChange()  {
  // Add your code here to act upon Minutes change
}
/*
  Since OverrideSend is READ_WRITE variable, onOverrideSendChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onOverrideSendChange()  {
  // Add your code here to act upon OverrideSend change
}
