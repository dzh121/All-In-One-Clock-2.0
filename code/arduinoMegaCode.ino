#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <FreeDefaultFonts.h>
#include "FreeSans20pt7b.h"
#include "FreeSans15pt7b.h"
#include "FreeSans10pt7b.h"
#include "FreeSans5pt7b.h"
#include <digital20pt7b.h>
#include <SoftwareSerial.h>
#include "icons.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>             // f.k. for Arduino-1.5.2
#define USE_SDFAT
#include <SdFat.h>           // Use the SdFat library

SoftSpiDriver<12, 11, 13> softSpi; //Bit-Bang on the Shield pins SDFat.h v2
SdFat SD;
#define SD_CS SdSpiConfig(10, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)

#define NAMEMATCH ""         // "" matches any name
#define PALETTEDEPTH   8     // support 256-colour Palette

char namebuf[32] = "/";   //BMP files in root directory

int Orientation = 1;

int temperature;
int humidity;
int pressure;
int feelsLike;
int UV;
float windSpeed;

bool reloadTemp = false;
bool reloadForecast = false;

int hours;
int minutes;
int month;
int year;
int dayMonth;
int dayWeek = 1;

int oldMin;
int oldDayMonth;

int imageNumber = 113;
String timeOfDayForImageToday = "day";

int todayMax;
int todayMin;
int todayRain;
int todayIcon;

int tommorowMax;
int tommorowMin;
int tommorowRain;
int tommorowIcon;

int tommorow1Max;
int tommorow1Min;
int tommorow1Rain;
int tommorow1Icon;

int alarmHours = 20;
int alarmMinutes = 0;

int oldDayMM;

bool alarmOn = false;

#define MINPRESSURE 50
#define MAXPRESSURE 1000

const int XP=9,XM=A3,YP=A2,YM=8; //320x480 ID=0x9488
const int TS_LEFT=120,TS_RT=900,TS_TOP=49,TS_BOT=949;
char myData[30];
char *ptr;

enum Screen {
  MAIN,
  ALARM,
  TEMP,
  FORECAST,
  ALARM_ACTIVE
};

Screen currentScreen = MAIN;

MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button temp_button, alarm_button, back_button, forecast_button, hour_up, minutes_up,dismissButton;

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x8410

bool canBeOnDelay = true;
int buzzer = 44;
bool alarmScreenShown = false;
bool reloadTempImage = false;
bool wasScreenBack = true;
unsigned long previousMillisAlarm = 0;
const long intervalAlarm = 2500; 
bool soundOn = false;

int pixel_x, pixel_y;
bool Touch_getXY(void)
{
    TSPoint tp = ts.getPoint();
    pinMode(YP, OUTPUT);      
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   
    digitalWrite(XM, HIGH);
    bool pressed = (tp.z > MINPRESSURE && tp.z < MAXPRESSURE);
    if (pressed) {
        switch (Orientation) {
            case 0:
                pixel_x = map(tp.x, TS_LEFT, TS_RT, 0, tft.width());
                pixel_y = map(tp.y, TS_TOP, TS_BOT, 0, tft.height());
                break;
            case 1:
                pixel_x = map(tp.y, TS_TOP, TS_BOT, 0, tft.width());
                pixel_y = map(tp.x, TS_RT, TS_LEFT, 0, tft.height());
                break;
            case 2:
                pixel_x = map(tp.x, TS_RT, TS_LEFT, 0, tft.width());
                pixel_y = map(tp.y, TS_BOT, TS_TOP, 0, tft.height());
                break;
            case 3:
                pixel_x = map(tp.y, TS_BOT, TS_TOP, 0, tft.width());
                pixel_y = map(tp.x, TS_LEFT, TS_RT, 0, tft.height());
                break;
        }
    }
    
    return pressed;
}

void receiveEvent(int numBytes);

void setup() {
  Serial.begin(9600);
  bool good = SD.begin(SD_CS);
  if (!good) {
      Serial.print(F("cannot start SD"));
      while (1);
  }
  pinMode(buzzer,OUTPUT);
  Wire.begin(8); // address of the Mega
  Wire.onReceive(receiveEvent); // set the receive event
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(Orientation );
  tft.fillScreen(BLACK);

  forecast_button.initButton(&tft, 382, 262, 150, 100, BLACK, BLACK, BLACK, "", 1);
  temp_button.initButton(&tft, 232, 262, 150, 100, BLACK, BLACK, BLACK, "", 1);
  alarm_button.initButton(&tft, 82, 262, 150, 100, BLACK, BLACK, BLACK, "", 1); 
  back_button.initButton(&tft, 430, 40, 96, 96, BLACK, BLACK, BLACK, "", 1);
  
  hour_up.initButton(&tft, 150, 230, 96, 96, WHITE, WHITE, WHITE, "", 2);
  minutes_up.initButton(&tft, 270, 230, 96, 96, WHITE, WHITE, WHITE, "", 2);
  dismissButton.initButton(&tft, 235, 245, 211, 92, BLACK, BLACK, BLACK, "", 2);
  
  forecast_button.drawButton(false);
  temp_button.drawButton(false);
  alarm_button.drawButton(false);

  defualtMain();  
}

void loop() {
  if((alarmHours != hours || alarmMinutes != minutes) &&!wasScreenBack) {
    wasScreenBack = true;
    noTone(buzzer);
    tft.fillScreen(BLACK);
    defualtMain();
  }
  
  if(alarmHours == hours && alarmMinutes == minutes && canBeOnDelay && currentScreen != ALARM){
    alarmActivesScreen();
  }else{
    switch(currentScreen) {
      case MAIN:
          mainScreen();
          break;
      case ALARM:
          alarmScreen();
          break;
      case TEMP:
          tempScreen();
          break;
      case FORECAST:
          forecastScreen();
          break;
    }    
  }
  
  if(alarmMinutes != minutes){
    canBeOnDelay = true;
    alarmScreenShown = false;
  }
  
}
void mainScreen(){
  bool down = Touch_getXY();
  temp_button.press(down && temp_button.contains(pixel_x, pixel_y));
  alarm_button.press(down && alarm_button.contains(pixel_x, pixel_y));
  forecast_button.press(down && forecast_button.contains(pixel_x, pixel_y));
  if(temp_button.justPressed()){
    tft.fillScreen(BLACK);
    Serial.println("pressed temp");
    currentScreen = TEMP;
    back_button.drawButton(false);
    tft.drawRGBBitmap(390, 5, homeScreen, 80, 80);
    reloadTemp = false;
    defaultTemp();
    return;
  }
  if(alarm_button.justPressed()){
    Serial.println("pressed alarm");
    tft.fillScreen(BLACK);
    currentScreen = ALARM;
    tft.setFont(&FreeSans15pt7b); 
    back_button.drawButton(false);
    hour_up.drawButton(false);
    minutes_up.drawButton(false);
    defaultAlarm();
    
    return;
  }
  if(forecast_button.justPressed()){
    Serial.println("pressed forecast");
    tft.fillScreen(BLACK);
    currentScreen = FORECAST;
    back_button.drawButton(false);
    reloadForecast = true;
    defaultForecast();
    tft.drawRGBBitmap(390, 5, homeScreen, 80, 80);
    showBMP("/arrow_up_icon.bmp", 10, 160);
    showBMP("/arrow_down_icon.bmp", 10, 200);
    showBMP("/rainChanceIcon.bmp", 10, 240);

    showBMP("/arrow_up_icon.bmp", 130, 160);
    showBMP("/arrow_down_icon.bmp", 130, 200);
    showBMP("/rainChanceIcon.bmp", 130, 240);

    showBMP("/arrow_up_icon.bmp", 250, 160);
    showBMP("/arrow_down_icon.bmp", 250, 200);
    showBMP("/rainChanceIcon.bmp", 250, 240);  
    return;
  }
  //image+temp
  if(reloadTempImage){
    tft.fillRect(15,0,100,120,BLACK);
    displayImage(timeOfDayForImageToday,imageNumber,15,50);
    tft.setFont(&FreeSans15pt7b); 
    tft.setTextSize(1); 
    tft.setCursor(20, 30);
    tft.setTextColor(WHITE);
    tft.print("T: ");
    tft.print(String(temperature));
    tft.print(" C"); 
    reloadTempImage = false;
  }
  
  //date
  if(oldDayMonth != dayMonth){
    tft.fillRect(225,0,240,50,BLACK);
    tft.setCursor(230, 30);
    tft.setTextSize(1); 
    tft.setFont(&FreeSans15pt7b); 
    tft.setTextColor(WHITE);
    tft.print(getDayOfWeek(dayWeek)+"  "+String(dayMonth / 10) + String(dayMonth % 10)+"."+String(month / 10) + String(month % 10)+"."+String(year));
    oldDayMonth = dayMonth;    
  }
  
  
  //hours:minutes
  if(oldMin != minutes){
    tft.fillRect(155,100,200,90,BLACK);
    tft.setCursor(160,160);
    tft.setTextColor(GREEN);
    tft.setTextSize(2.9);
    tft.setFont(&digital20pt7b);
    tft.print(String(hours/10)+String(hours%10)+":"+String(minutes/10)+String(minutes%10));
    tft.setFont(&FreeSans15pt7b);
    tft.setTextColor(WHITE);
    oldMin = minutes;
  }
  
}
void alarmScreen(){
  bool down = Touch_getXY();
  back_button.press(down && back_button.contains(pixel_x, pixel_y));
  hour_up.press(down && hour_up.contains(pixel_x, pixel_y));
  minutes_up.press(down && minutes_up.contains(pixel_x, pixel_y));
  if(back_button.justPressed()){
    Serial.println("pressed back");
    if(alarmHours == hours && alarmMinutes == minutes && canBeOnDelay){
      currentScreen = MAIN;
      alarmActivesScreen();
      return;
    }
    tft.fillScreen(BLACK);
    currentScreen = MAIN;
    defualtMain();
    return;
  }
  if(hour_up.justPressed()){
    alarmHours = (alarmHours + 1)% 24;
    tft.fillRect(105, 90,200,80,BLACK);
    tft.setFont(&digital20pt7b);
    tft.setTextColor(GREEN);
    tft.setCursor(120, 150);
    tft.setTextSize(2); 
    tft.print(String(alarmHours/10)+String(alarmHours%10)+":"+String(alarmMinutes/10)+String(alarmMinutes%10));
  }
  if(minutes_up.justPressed()){
    canBeOnDelay = true;
    alarmMinutes = (alarmMinutes + 1)% 60;
    tft.fillRect(105, 90,200,80,BLACK);
    tft.setFont(&digital20pt7b);
    tft.setTextColor(GREEN);
    tft.setCursor(120, 150);
    tft.setTextSize(2); 
    tft.print(String(alarmHours/10)+String(alarmHours%10)+":"+String(alarmMinutes/10)+String(alarmMinutes%10));    
  }
  
  
}
void tempScreen(){
  bool down = Touch_getXY();
  back_button.press(down && back_button.contains(pixel_x, pixel_y));
  if(back_button.justPressed()){
    Serial.println("pressed back");
    tft.fillScreen(BLACK);
    currentScreen = MAIN;
    defualtMain();
    return;
  }

  displayImage(timeOfDayForImageToday,imageNumber,390,250);
  if(reloadTemp){
    tft.fillRect(30,70,90,40,BLACK);
    tft.fillRect(160,70,90,40,BLACK);
    tft.fillRect(270,70,120,40,BLACK);
    tft.fillRect(30,200,90,40,BLACK);
    tft.fillRect(160,200,35,40,BLACK);
    tft.fillRect(230,200,160,40,BLACK);
    defaultTemp();
    reloadTemp = false;
  }
  

}
void forecastScreen(){
  bool down = Touch_getXY();
  back_button.press(down && back_button.contains(pixel_x, pixel_y));
  if(back_button.justPressed()){
    Serial.println("pressed back");
    tft.fillScreen(BLACK);
    defualtMain();
    currentScreen = MAIN;
    return;
  }
  
  if(reloadForecast){
    tft.fillRect(45,20,70,250,BLACK);
    tft.fillRect(165,20,70,250,BLACK);
    tft.fillRect(285,20,70,250,BLACK);
    defaultForecast();
    reloadForecast = false;
  }
  
}
void alarmActivesScreen(){
  wasScreenBack = false;
   static unsigned long lastBuzzTime = 0;
   if (!alarmScreenShown) {
    tft.fillScreen(BLACK);
    defaultAlarmActive();
    alarmScreenShown = true;
  }
    
  unsigned long currentMillisAlarm = millis();
  if (currentMillisAlarm - previousMillisAlarm >= intervalAlarm) {
    previousMillisAlarm = currentMillisAlarm;
    if(soundOn){
      soundOn = false;
      noTone(buzzer);
    }else{
      soundOn = true;
      analogWrite(buzzer, 200);
    }
  }

  bool down = Touch_getXY();
  dismissButton.press(down && dismissButton.contains(pixel_x, pixel_y));
  if(dismissButton.justPressed()){
    Serial.println("pressed dismiss");
    tft.fillScreen(BLACK);
    currentScreen = MAIN;
    noTone(buzzer);
    defualtMain();
    canBeOnDelay = false;
    wasScreenBack = true;
    return;
  }
  
}

String getDayOfWeek(int dayOfWeek) {
  String dayName; // Initialize empty string for day name
  if(dayOfWeek > 7){
    dayOfWeek -= 7;
  }
  switch(dayOfWeek) {
    case 1:
      dayName = "Sun";
      break;
    case 2:
      dayName = "Mon";
      break;
    case 3:
      dayName = "Tue";
      break;
    case 4:
      dayName = "Wed";
      break;
    case 5:
      dayName = "Thu";
      break;
    case 6:
      dayName = "Fri";
      break;
    case 7:
      dayName = "Sat";
      break;
    default:
      dayName = ""; // Invalid day of week integer
      break;
  }

  return dayName;
}

void displayImage(String timeOfDay,int imageNumber, int x, int y) {
  String filename = "/"+timeOfDay+"/"+String(imageNumber)+".bmp";
  showBMP(filename.c_str(), x, y);
}

void defaultForecast(){
  Serial.println(tommorowIcon);
  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans15pt7b);
  tft.setTextSize(1); 
 
  //today
  tft.setCursor(50, 60);
  tft.print(getDayOfWeek(dayWeek));
  displayImage("day",todayIcon,40,80);
  tft.setCursor(50, 180);
  tft.print(String(todayMax)+"C");
  tft.setCursor(50, 220);
  tft.print(String(todayMin)+"C"); 
  tft.setCursor(50, 260);
  tft.print(String(todayRain)+"%"); 

  //tommorow 
  tft.setCursor(170, 60);
  tft.print(getDayOfWeek((dayWeek+1)));
  displayImage("day",tommorowIcon,160,80);
  tft.setCursor(170, 180);
  tft.print(String(tommorowMax)+"C");
  tft.setCursor(170, 220);
  tft.print(String(tommorowMin)+"C"); 
  tft.setCursor(170, 260);
  tft.print(String(tommorowRain)+"%"); 

  //the day after tommorow
  tft.setCursor(290, 60);
  tft.print(getDayOfWeek((dayWeek+2)));
  displayImage("day",tommorow1Icon,280,80);
  tft.setCursor(290, 180);
  tft.print(String(tommorow1Max)+"C");
  tft.setCursor(290, 220);
  tft.print(String(tommorow1Min)+"C"); 
  tft.setCursor(290, 260);
  tft.print(String(tommorow1Rain)+"%"); 
}
void defualtMain(){
  tft.fillScreen(BLACK);
  tft.drawRGBBitmap(50, 230, alarmIcon, 64, 64);
  tft.drawRGBBitmap(200, 230, tempIcon, 64, 64);
  tft.drawRGBBitmap(350, 230, forecastIcon, 64, 64);
  displayImage(timeOfDayForImageToday,imageNumber,15,50);
  tft.fillRect(15,0,150,50,BLACK);
  tft.setFont(&FreeSans15pt7b); 
  tft.setTextSize(1); 
  tft.setCursor(20, 30);
  tft.setTextColor(WHITE);
  tft.print("T: ");
  tft.print(String(temperature));
  tft.print(" C"); 
  tft.fillRect(225,0,240,50,BLACK);
  tft.setCursor(230, 30);
  tft.setTextSize(1); 
  tft.setFont(&FreeSans15pt7b); 
  tft.setTextColor(WHITE);
  tft.print(getDayOfWeek(dayWeek)+"  "+String(dayMonth / 10) + String(dayMonth % 10)+"."+String(month / 10) + String(month % 10)+"."+String(year));
  tft.fillRect(155,100,200,90,BLACK);
  tft.setCursor(160,160);
  tft.setTextColor(GREEN);
  tft.setTextSize(2.9);
  tft.setFont(&digital20pt7b);
  tft.print(String(hours/10)+String(hours%10)+":"+String(minutes/10)+String(minutes%10));
  tft.setTextColor(WHITE);
  tft.setTextSize(1); 
}
void defaultTemp(){
  tft.setTextSize(1);
  //Temperature
  tft.setCursor(20, 60);
  tft.setTextColor(GRAY);
  tft.setFont(&FreeSans10pt7b); 
  tft.print("Temperature");
  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans20pt7b); 
  tft.setCursor(40, 105);
  tft.print(String(temperature)+"C"); 

  //humidity  
  tft.setCursor(160, 60);
  tft.setTextColor(GRAY);
  tft.setFont(&FreeSans10pt7b); 
  tft.print("Humidity");
  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans20pt7b); 
  tft.setCursor(160, 105);
  tft.print(String(humidity)+"%"); 

  //Pressure  
  tft.setCursor(270, 60);
  tft.setTextColor(GRAY);
  tft.setFont(&FreeSans10pt7b); 
  tft.print("Pressure");
  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans20pt7b); 
  tft.setCursor(270, 105);
  tft.print(pressure); 

  //Feels Like
  tft.setCursor(20, 190);
  tft.setTextColor(GRAY);
  tft.setFont(&FreeSans10pt7b); 
  tft.print("Feels Like");
  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans20pt7b); 
  tft.setCursor(35, 230);
  tft.print(String(feelsLike)+"C"); 

  //UV  
  tft.setCursor(160, 190);
  tft.setTextColor(GRAY);
  tft.setFont(&FreeSans10pt7b); 
  tft.print("UV");
  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans20pt7b); 
  tft.setCursor(165, 230);
  tft.print(UV); 

  //wind Speed  
  tft.setCursor(255, 190);
  tft.setTextColor(GRAY);
  tft.setFont(&FreeSans10pt7b); 
  tft.print("wind Speed");
  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans20pt7b); 
  tft.setCursor(230, 230);
  tft.print(String(windSpeed,1)+"KPH"); 
}
void defaultAlarm(){
  tft.fillRect(102,182,96,96,WHITE);
    tft.fillRect(105,185,90,90,BLACK);
    tft.setCursor(120, 255);
    tft.setFont(&FreeSans20pt7b);
    tft.setTextSize(2); 
    tft.print("H");
    tft.fillRect(222,182,96,96,WHITE);
    tft.fillRect(225,185,90,90,BLACK);
    tft.setCursor(240, 255);
    tft.setFont(&FreeSans20pt7b);
    tft.setTextSize(2); 
    tft.print("M");
    tft.drawRGBBitmap(390, 5, homeScreen, 80, 80);
    tft.setTextColor(WHITE);
    tft.setFont(&FreeSans15pt7b);
    tft.setTextSize(1); 
    tft.setCursor(150, 40);
    tft.print("Set Alarm");
    tft.setFont(&digital20pt7b);
    tft.setTextColor(GREEN);
    tft.setCursor(120, 150);
    tft.setTextSize(2); 
    tft.print(String(alarmHours/10)+String(alarmHours%10)+":"+String(alarmMinutes/10)+String(alarmMinutes%10));
}
void defaultAlarmActive(){
  dismissButton.drawButton(false);
  tft.setTextSize(1); 
  tft.setTextColor(WHITE);
  tft.setFont(&FreeSans20pt7b); 
  tft.drawRGBBitmap(180, 20, alarmIcon, 64, 64);
  tft.setCursor(150,140);
  tft.print("ALARM");
  tft.setCursor(150,200);
  tft.print(String(alarmHours/10)+String(alarmHours%10)+":"+String(alarmMinutes/10)+String(alarmMinutes%10));
  tft.fillRect(150,220,174,48,WHITE);
  tft.fillRect(153,223,168,42,BLACK);
  tft.setCursor(156, 255);
  tft.print("DISMISS");

}
void receiveEvent(int numBytes) {
  StaticJsonDocument<200> doc;
  char data[numBytes];
  int i = 0;
  while (Wire.available()) {
    char c = Wire.read(); // read the incoming data
    data[i] = c;
    i++;
    Serial.write(c);
  }
  Serial.println("");
  DeserializationError error = deserializeJson(doc, data);
  if (error) {
    Serial.println("deserializeJson() failed");
  } else {
    temperature = doc.containsKey("tw") ? doc["tw"] : temperature;
    humidity = doc.containsKey("hw") ? doc["hw"] : humidity;
    pressure = doc.containsKey("pw") ? doc["pw"] : pressure;
    hours = doc.containsKey("ht") ? doc["ht"] : hours;
    minutes = doc.containsKey("mit") ? doc["mit"] : minutes;
    month = doc.containsKey("mot") ? doc["mot"] : month;
    year = doc.containsKey("yt") ? doc["yt"] : year;
    dayMonth = doc.containsKey("dmt") ? doc["dmt"] : dayMonth;
    dayWeek = doc.containsKey("dwt") ? doc["dwt"] : dayWeek;
    imageNumber = doc.containsKey("int") ? doc["int"] : imageNumber;
    timeOfDayForImageToday = doc.containsKey("nort") ? doc["nort"] : timeOfDayForImageToday;
    feelsLike = doc.containsKey("flt") ? doc["flt"] : feelsLike;
    UV = doc.containsKey("uv") ? doc["uv"] : UV;
    windSpeed = doc.containsKey("wst") ? doc["wst"] : windSpeed;

    todayMax = doc.containsKey("t1mx") ? doc["t1mx"] : todayMax;
    todayMin = doc.containsKey("t1mi") ? doc["t1mi"] : todayMin;
    todayRain = doc.containsKey("t1r") ? doc["t1r"] : todayRain;
    todayIcon = doc.containsKey("t1c") ? doc["t1c"] : todayIcon;
    tommorowMax = doc.containsKey("t2mx") ? doc["t2mx"] : tommorowMax;
    tommorowMin = doc.containsKey("t2mi") ? doc["t2mi"] : tommorowMin;
    tommorowRain = doc.containsKey("t2r") ? doc["t2r"] : tommorowRain;
    tommorowIcon = doc.containsKey("t2c") ? doc["t2c"] : tommorowIcon;
    tommorow1Max = doc.containsKey("t3mx") ? doc["t3mx"] : tommorow1Max;
    tommorow1Min = doc.containsKey("t3mi") ? doc["t3mi"] : tommorow1Min;
    tommorow1Rain = doc.containsKey("t3r") ? doc["t3r"] : tommorow1Rain;
    tommorow1Icon = doc.containsKey("t3c") ? doc["t3c"] : tommorow1Icon;
    
    if(doc.containsKey("wst") || doc.containsKey("uv") || doc.containsKey("flt") || doc.containsKey("tw") || doc.containsKey("hw") || doc.containsKey("pw")){
      reloadTemp = true;
    }
    if (doc.containsKey("t1mx") || doc.containsKey("t1mi") || doc.containsKey("t1r") || doc.containsKey("t1c") ||
    doc.containsKey("t2mx") || doc.containsKey("t2mi") || doc.containsKey("t2r") || doc.containsKey("t2c") ||
    doc.containsKey("t3mx") || doc.containsKey("t3mi") || doc.containsKey("t3r") || doc.containsKey("t3c") || oldDayMM != dayMonth) {
      reloadForecast = true;
    }
  
  if(doc.containsKey("int") || doc.containsKey("tw")){
    reloadTempImage = true;
  }
    oldDayMM = dayMonth;
  }
}

#define BMPIMAGEOFFSET 54
#define BUFFPIXEL      40
uint16_t read16(File& f) {
    uint16_t result;         // read little-endian
    f.read(&result, sizeof(result));
    return result;
}

uint32_t read32(File& f) {
    uint32_t result;
    f.read(&result, sizeof(result));
    return result;
}

uint8_t showBMP(char *nm, int x, int y)
{
    File bmpFile;
    int bmpWidth, bmpHeight;    // W+H in pixels
    uint8_t bmpDepth;           // Bit depth (currently must be 24, 16, 8, 4, 1)
    uint32_t bmpImageoffset;    // Start of image data in file
    uint32_t rowSize;           // Not always = bmpWidth; may have padding
    uint8_t sdbuffer[3 * BUFFPIXEL];    // pixel in buffer (R+G+B per pixel)
    uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
    uint8_t bitmask, bitshift;
    boolean flip = true;        // BMP is stored bottom-to-top
    int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
    uint32_t pos;               // seek position
    boolean is565 = false;      //

    uint16_t bmpID;
    uint16_t n;                 // blocks read
    uint8_t ret;

    if ((x >= tft.width()) || (y >= tft.height()))
        return 1;               // off screen

    bmpFile = SD.open(nm);      // Parse BMP header
    bmpID = read16(bmpFile);    // BMP signature
    (void) read32(bmpFile);     // Read & ignore file size
    (void) read32(bmpFile);     // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile);       // Start of image data
    (void) read32(bmpFile);     // Read & ignore DIB header size
    bmpWidth = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    n = read16(bmpFile);        // # planes -- must be '1'
    bmpDepth = read16(bmpFile); // bits per pixel
    pos = read32(bmpFile);      // format
    if (bmpID != 0x4D42) ret = 2; // bad ID
    else if (n != 1) ret = 3;   // too many planes
    else if (pos != 0 && pos != 3) ret = 4; // format: 0 = uncompressed, 3 = 565
    else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH) ret = 5; // palette 
    else {
        bool first = true;
        is565 = (pos == 3);               // ?already in 16-bit format
        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
        if (bmpHeight < 0) {              // If negative, image is in top-down order.
            bmpHeight = -bmpHeight;
            flip = false;
        }

        w = bmpWidth;
        h = bmpHeight;
        if ((x + w) >= tft.width())       // Crop area to be loaded
            w = tft.width() - x;
        if ((y + h) >= tft.height())      //
            h = tft.height() - y;

        if (bmpDepth <= PALETTEDEPTH) {   // these modes have separate palette
            //bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
            bmpFile.seek(bmpImageoffset - (4<<bmpDepth)); //54 for regular, diff for colorsimportant
            bitmask = 0xFF;
            if (bmpDepth < 8)
                bitmask >>= bmpDepth;
            bitshift = 8 - bmpDepth;
            n = 1 << bmpDepth;
            lcdbufsiz -= n;
            palette = lcdbuffer + lcdbufsiz;
            for (col = 0; col < n; col++) {
                pos = read32(bmpFile);    //map palette to 5-6-5
                palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
            }
        }

        // Set TFT address window to clipped image bounds
        tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
        for (row = 0; row < h; row++) { // For each scanline...
            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            uint8_t r, g, b, *sdptr;
            int lcdidx, lcdleft;
            if (flip)   // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
            else        // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize;
            if (bmpFile.position() != pos) { // Need seek?
                bmpFile.seek(pos);
                buffidx = sizeof(sdbuffer); // Force buffer reloadTemp
            }

            for (col = 0; col < w; ) {  //pixels in row
                lcdleft = w - col;
                if (lcdleft > lcdbufsiz) lcdleft = lcdbufsiz;
                for (lcdidx = 0; lcdidx < lcdleft; lcdidx++) { // buffer at a time
                    uint16_t color;
                    // Time to read more pixel data?
                    if (buffidx >= sizeof(sdbuffer)) { // Indeed
                        bmpFile.read(sdbuffer, sizeof(sdbuffer));
                        buffidx = 0; // Set index to beginning
                        r = 0;
                    }
                    switch (bmpDepth) {          // Convert pixel from BMP to TFT format
                        case 32:
                        case 24:
                            b = sdbuffer[buffidx++];
                            g = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (bmpDepth == 32) buffidx++; //ignore ALPHA
                            color = tft.color565(r, g, b);
                            break;
                        case 16:
                            b = sdbuffer[buffidx++];
                            r = sdbuffer[buffidx++];
                            if (is565)
                                color = (r << 8) | (b);
                            else
                                color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
                            break;
                        case 1:
                        case 4:
                        case 8:
                            if (r == 0)
                                b = sdbuffer[buffidx++], r = 8;
                            color = palette[(b >> bitshift) & bitmask];
                            r -= bmpDepth;
                            b <<= bmpDepth;
                            break;
                    }
                    lcdbuffer[lcdidx] = color;

                }
                tft.pushColors(lcdbuffer, lcdidx, first);
                first = false;
                col += lcdidx;
            }           // end cols
        }               // end rows
        tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
        ret = 0;        // good render
    }
    bmpFile.close();
    return (ret);
}
