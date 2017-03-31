/**The MIT License (MIT)
Copyright (c) 2015 by Daniel Eichhorn
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
See more at http://blog.squix.ch
*/



#include <Arduino.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Wire.h>  // required even though we do not use I2C 
#include "Adafruit_STMPE610.h"

// Fonts created by RKG
#include "fonts.h"

// Download helper
#include "WebResource.h"

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// Helps with connecting to internet
#include <WiFiManager.h>

// check settings.h for adapting to your needs
#include "settings.h"
#include <JsonListener.h>
#include <WundergroundClient.h>
#include "TimeClient.h"

#include "embHTML.h"

// HOSTNAME for OTA update
#define HOSTNAME "WSC-ESP8266-OTA-"

/* Fonts */
#define font_Tahoma22 201
#define font_Comic40 202
#define font_System10 203

/*****************************
 * display
 * ***************************/
#define LCDWidth 176  //define screen width,height
#define LCDHeight 220
#define _Digole_Serial_I2C_  //To tell compiler compile the special communication only, 
#define Ver 34           //if the version of firmware on display is V3.3 and newer, use this

#define WHITE 0xFF
#define BLACK 0
#define RED 0xA6
#define GREEN 0x55
#define BLUE 0x97

#include <DigoleSerial.h> // Hardware-specific library
#include <Wire.h>
DigoleSerialDisp  tft(&Wire, '\x27'); 

/*****************************
 * Important: see settings.h to configure your settings!!!
 * ***************************/
// Additional UI functions
#include "GfxUi.h"

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
GfxUi ui = GfxUi(&tft);

Adafruit_STMPE610 spitouch = Adafruit_STMPE610(STMPE_CS);

WebResource webResource;
TimeClient timeClient(UTC_OFFSET);

// Set to false, if you prefere imperial/inches, Fahrenheit
WundergroundClient wunderground(IS_METRIC);

//declaring prototypes
void configModeCallback (WiFiManager *myWiFiManager);
void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal);
ProgressCallback _downloadCallback = downloadCallback;
void downloadResources();
void updateData();
void drawProgress(uint8_t percentage, String text);
void drawTime();
void drawCurrentWeather();
void drawForecast();
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex);
String getMeteoconIcon(String iconText);
void drawAstronomy();
void drawSeparator(uint16_t y);
void sleepNow(int wakeup);
void installFonts();

long lastDownloadUpdate = millis();


//WiFiManager
//Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wifiManager;
ESP8266WebServer server(80);

/* webserver handlers */

void handle_root()
{
  String content = FPSTR(PAGE_INDEX);
  content.replace("{country}", WUNDERGROUND_COUNTRY);
  content.replace("{city}", WUNDERGROUND_CITY);
  content.replace("{timeoffset}",String(UTC_OFFSET).c_str());
  server.send(200, "text/html", content);
}

static bool forceUpdateData = false;
void handle_store_settings(){
  if(server.arg("_country")==NULL && server.arg("_city")==NULL && server.arg("_timeoffset")==NULL){
    Serial.println("setting page refreshed only, no params");      
  }else{
    Serial.println("Location changed");  
    WUNDERGROUND_COUNTRY = server.arg("_country");
    WUNDERGROUND_COUNTRY.replace(" ","_");
    Serial.println("Coutry: " + WUNDERGROUND_COUNTRY);
    WUNDERGROUND_CITY = server.arg("_city");
    WUNDERGROUND_CITY.replace(" ","_") ;
    Serial.println("City: " + WUNDERGROUND_CITY);
    UTC_OFFSET = atof(server.arg("_timeoffset").c_str());
    Serial.print("TimeOffset: "); Serial.println(UTC_OFFSET);
  
    Serial.println("writing custom setting start");
    Serial.println("file: " + CUSTOM_SETTINGS);
    //write location to SPIFF
    File f = SPIFFS.open(CUSTOM_SETTINGS, "w");
    if (f){
      f.print(WUNDERGROUND_COUNTRY.c_str()); f.print("\r");
      Serial.println("Country: " + WUNDERGROUND_COUNTRY);
      f.print(WUNDERGROUND_CITY.c_str()); f.print("\r");
      Serial.println("City: " + WUNDERGROUND_CITY);
      f.print(UTC_OFFSET); f.print("\r");
      Serial.print("Offset: "); Serial.println(UTC_OFFSET);
    }else{
      Serial.println("file open failed: " + CUSTOM_SETTINGS);
    }
    f.flush();
    f.close();
    Serial.println("writing custom setting end");
    
    //updateData(false);
    forceUpdateData = true;
  }
  timeClient.setTimeOffset(UTC_OFFSET);
  server.send(200, "text/html", "OK");
}

void read_custom_settings(){
    //read setting from SPIFF
    Serial.println("reading custom setting start");
    File f = SPIFFS.open(CUSTOM_SETTINGS, "r");
    if(f){
      int i=0;
      char ch;
  
      //WUNDERGROUND_COUNTRY
      char country[64]; i=0;
      while((ch=f.read())!='\r'){
        country[i++]=ch;
      } WUNDERGROUND_COUNTRY = String(country);
      Serial.println("Country: " + WUNDERGROUND_COUNTRY);
  
      //WUNDERGROUND_CITY
      char city[64]; i=0;
      while((ch=f.read())!='\r'){
        city[i++]=ch;
      } WUNDERGROUND_CITY = String(city);
      Serial.println("City: " + WUNDERGROUND_CITY);
      
      //UTC_OFFSET
      char offset[5]; i=0;
      while((ch=f.read())!='\r'){
        offset[i++]=ch;
      } UTC_OFFSET = atof(offset);
      Serial.print("Offset: "); Serial.println(UTC_OFFSET);
    }else{
      Serial.println("file open failed: " + CUSTOM_SETTINGS);
    }
    f.close();
    Serial.println("reading custom setting end");

    timeClient.setTimeOffset(UTC_OFFSET);
}
/**/
  
void setup() {
  pinMode(TFT_POWER_PIN, OUTPUT);
  digitalWrite(TFT_POWER_PIN, LOW);
  
  Wire.begin(TFT_I2C_DATA, TFT_I2C_CLOCK);//mcu esp8266 
  Serial.begin(9600);
  
  tft.begin();
  delay(100);
  digitalWrite(TFT_POWER_PIN, HIGH);

  tft.setMode('C');
  // wipe screen & backlight on
  tft.setDrawWindow(0, 0, 176, 220);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow(); //clear draw window use the new back ground color
  tft.backLightOn();
//  tft.backLightBrightness(50);

  tft.setFont(10);
  tft.setColor(WHITE);
  ui.setTextAlignment(CENTER);
  ui.drawString(10, 200, "Connecting to WiFi");

  // Uncomment for testing wifi manager
  //wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  //Manual Wifi
  //WiFi.begin(WIFI_SSID, WIFI_PWD);


  // OTA Setup
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.begin();
  SPIFFS.begin();
  read_custom_settings();   

  //user setting handling
  server.on("/", handle_root);
  server.on("/loc", handle_store_settings);
  server.begin(); 
  Serial.println("HTTP server started"); 
  
  //Uncomment if you want to update all internet resources
  //SPIFFS.format();

  //Uncomment if you want to reinstall all custom Fonts
  //installFonts();


  // Incomment if you want to download images from the net. If images already exist don't download
  //downloadResources();

  // load the weather information

//  digitalWrite(TFT_POWER_PIN, LOW);
//  delay(100);
//  digitalWrite(TFT_POWER_PIN, HIGH);

  updateData();
}

long lastDrew = 0;
long stamp = 0;
void loop() {
  // "standard setup"
  {
 
    // Handle web server
    server.handleClient();

    // Handle OTA update requests
    ArduinoOTA.handle();


    // Check if we should update the clock
    if (stamp - lastDrew > 30000 && wunderground.getSeconds() == "00" || stamp <= lastDrew || forceUpdateData ) {
      drawTime(false);
      stamp = millis();
    }

    // Check if we should update weather information
    if (stamp - lastDownloadUpdate > 1000 * UPDATE_INTERVAL_SECS || stamp <= lastDownloadUpdate || forceUpdateData) {
      updateData(false);
      lastDownloadUpdate = millis();
    }

    if(forceUpdateData) forceUpdateData = false;
  }
}

// Called if WiFi has not been configured yet
void configModeCallback (WiFiManager *myWiFiManager) {
  ui.setTextAlignment(CENTER);
  tft.setFont(6);
  tft.setColor(BLUE);
  ui.drawString(10, 28, "Wifi Manager");
  ui.drawString(10, 42, "Please connect to AP");
  tft.setColor(WHITE);
  ui.drawString(10, 56, myWiFiManager->getConfigPortalSSID());
  tft.setColor(BLUE);
  ui.drawString(10, 70, "To setup Wifi Configuration");
}

// callback called during download of files. Updates progress bar
void downloadCallback(String filename, int16_t bytesDownloaded, int16_t bytesTotal) {
  Serial.println(String(bytesDownloaded) + " / " + String(bytesTotal));

  int percentage = 100 * bytesDownloaded / bytesTotal;
  if (percentage == 0) {
    ui.drawString(10,160,filename);
  }
  if (percentage % 5 == 0) {
    ui.setTextAlignment(CENTER);
    tft.setColor(BLUE);
    ui.drawString(10,160,String(percentage) + "%");
    ui.drawProgressBar(10, 165, 156 , 15, percentage, WHITE, BLUE);
  }

}

// Download the bitmaps
void downloadResources() {
  tft.setDrawWindow(0, 0, 176, 220);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow(); //clear draw window use the new back ground color

  tft.setFont(6);
  char id[5];
  for (int i = 0; i < 21; i++) {
    sprintf(id, "%02d", i);
    tft.setColor(BLUE);
    tft.drawBox(0, 120, 176/20*(i+1), 40);
    tft.setColor(RED|BLUE);
    ui.drawString(10,138,String(id)+" "+wundergroundIcons[i]);
    webResource.downloadFile("http://www.squix.org/blog/wunderground/" + wundergroundIcons[i] + ".bmp", wundergroundIcons[i] + ".bmp", _downloadCallback);
  }
  for (int i = 0; i < 21; i++) {
    sprintf(id, "%02d", i);
    tft.setColor(RED);
    tft.drawBox(0, 120, 176/20*(i+1), 40);
    tft.setColor(RED|BLUE);
    ui.drawString(10,138,String(id)+" mini "+wundergroundIcons[i]);
    webResource.downloadFile("http://www.squix.org/blog/wunderground/mini/" + wundergroundIcons[i] + ".bmp", "/mini/" + wundergroundIcons[i] + ".bmp", _downloadCallback);
  }
  for (int i = 0; i < 23; i++) {
    sprintf(id, "%02d", i);
    tft.setColor(WHITE);
    tft.drawBox(0, 120, 176/22*(i+1), 40);
    tft.setColor(RED|BLUE);
    ui.drawString(10,138,String(id)+" moon ");
    webResource.downloadFile("http://www.squix.org/blog/moonphase_L" + String(i) + ".bmp", "/moon" + String(i) + ".bmp", _downloadCallback);
  }
}

/********************************************************************************************************/
void installFonts(){
  tft.setDrawWindow(0, 0, 176, 220);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow();
  tft.setColor(WHITE);
  tft.setFont(10);
  tft.drawStr(0, 0, "Installing Fonts");
  Serial.println("Installing Fonts");
  tft.drawStr(0, 1, "Tahoma:");
  delay(1000); //This delay is very important, it will let the module clean the receiving buffer,then accept bulk data bellow
  tft.downloadUserFont(sizeof(tahoma22), tahoma22, 1); //download a user font: (font length, font address, #userfont), one time download needed
  delay(500);
  tft.drawStr(0, 2, "Comic:");
  delay(1000); //This delay is very important, it will let the module clean the receiving buffer,then accept bulk data bellow
  tft.downloadUserFont(sizeof(comic40), comic40, 2); //download a user font: (font length, font address, #userfont), one time download needed
  delay(500);
  tft.drawStr(0, 3, "System:");
  delay(1000); //This delay is very important, it will let the module clean the receiving buffer,then accept bulk data bellow
  tft.downloadUserFont(sizeof(sysfont12), sysfont12, 3); //download a user font: (font length, font address, #userfont), one time download needed
  delay(500);
  tft.drawStr(0, 4, "OK");
  delay(500);
  tft.cleanDrawWindow();
}

// Update the internet based information and update screen
void updateData() { updateData(true); }
void updateData(bool visual) {
  tft.setDrawWindow(0, 0, 176, 220);  
  tft.setBgColor(BLACK);
  if(visual){
    tft.cleanDrawWindow(); //clear draw window use the new back ground color
    tft.setColor(WHITE);
    tft.setFont(font_System10);
    drawProgress(20, "Updating time...");
  }
  timeClient.updateTime();
  if(visual) drawProgress(50, "Updating conditions...");
  wunderground.updateConditions(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  if(visual) drawProgress(70, "Updating forecasts...");
  wunderground.updateForecast(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  if(visual) drawProgress(90, "Updating astronomy...");
  wunderground.updateAstronomy(WUNDERGRROUND_API_KEY, WUNDERGRROUND_LANGUAGE, WUNDERGROUND_COUNTRY, WUNDERGROUND_CITY);
  //lastUpdate = timeClient.getFormattedTime();
  //readyForWeatherUpdate = false;
  if(visual) drawProgress(100, "Done................");
  delay(1000);
  
  if(visual){
    tft.setDrawWindow(0, 0, 176, 220);  
    tft.setBgColor(BLACK);
    tft.cleanDrawWindow(); //clear draw window use the new back ground color
  }
  
  drawTime();
  drawCurrentWeather();
  drawForecast();
  drawAstronomy();
}

// Progress bar helper
void drawProgress(uint8_t percentage, String text) {
  tft.setColor(BLACK);
  tft.drawBox(0, 140, 240, 45);
  tft.setColor(WHITE);
  tft.setFont(6);
  ui.drawString(10,186,text);
  ui.drawProgressBar(10, 165, 156, 15, percentage, WHITE, BLUE);
}

// draws the clock
void drawTime() { drawTime(true); }
void drawTime(bool clear) {
  tft.setDrawWindow(0, 0, 176, 55);  
  tft.setBgColor(BLACK);
  if(clear) 
     tft.cleanDrawWindow(); //clear draw window use the new back ground color
  
  ui.setTextAlignment(CENTER);
  tft.setFont(font_System10);
  String date = wunderground.getDate();
  tft.setColor(WHITE);
  ui.drawString(38,15,date);
  
  tft.setColor(BLUE);
  tft.setFont(font_Comic40);
  String time = "  "+timeClient.getHours() + ":" + timeClient.getMinutes()+"  ";
  ui.drawString(25,54,time);
//  drawSeparator(55);
}

// draws current weather information
void drawCurrentWeather() {
  tft.setDrawWindow(0, 45, 176, 90);  
  tft.setBgColor(BLACK);

  // Weather Icon
  tft.setDrawWindow(0, 45, 88, 90);  
  String weatherIcon = getMeteoconIcon(wunderground.getTodayIcon());
  ui.drawBmp(weatherIcon + ".bmp", 0, 0);
  
  tft.setDrawWindow(88, 45, 88, 90);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow(); //clear draw window use the new back ground color

  // City
  tft.setFont(6);
  tft.setColor(WHITE);
  ui.setTextAlignment(RIGHT);
  ui.drawString(95, 13, WUNDERGROUND_CITY);
  
  // Weather Text
  tft.setFont(font_System10);
  tft.setColor(WHITE);
  ui.setTextAlignment(RIGHT);
  ui.drawString(95, 25, wunderground.getWeatherText());
  
  tft.setFont(/*font_Comic40*/font_Tahoma22);
  tft.setColor(BLUE);
  ui.setTextAlignment(RIGHT);
  char tmp[6];
  String temp = wunderground.getCurrentTemp();
  float t = atof(temp.c_str());
  Serial.print("atof: "); Serial.println(t);
  dtostrf(t, 4, 1, tmp);
  if(t>0) tft.setTrueColor(20+map(t,0,40,0,43),43,50); /*tft.setColor(RED);*/
  
  tft.setFont(font_Tahoma22);
  if(t>0) ui.drawString(88,44,"+");
  else if(t<0) ui.drawString(88,44,"-");
  
  dtostrf(abs(t), 4, 1, tmp);
  tft.setFont(font_Comic40);
  ui.drawString(98,37,tmp);

  tft.setMode('|');
  String degreeSign = IS_METRIC?"C":"F";
  tft.setFont(font_System10);
  ui.setTextAlignment(RIGHT);
  ui.drawString(160, 40, degreeSign);
  tft.setMode('C');
  
//  drawSeparator(125);
}

// draws the three forecast columns
void drawForecast() {
  tft.setDrawWindow(0, 130, 176, 70);  
  tft.setBgColor(BLACK);
//  tft.cleanDrawWindow(); //clear draw window use the new back ground color

  tft.setDrawWindow(10, 130, 65, 17);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow(); //clear draw window use the new back ground color
  tft.setDrawWindow(0, 130, 176, 70);  
  drawForecastDetail(10, 0, 0);

  tft.setDrawWindow(65, 130, 55, 17);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow(); //clear draw window use the new back ground color
  tft.setDrawWindow(0, 130, 176, 70);  
  drawForecastDetail(65, 0, 2);

  tft.setDrawWindow(120, 130, 60, 17);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow(); //clear draw window use the new back ground color
  tft.setDrawWindow(0, 130, 176, 70);  
  drawForecastDetail(120, 0, 4);
//  drawSeparator(210);
}

// helper for the forecast columns
void drawForecastDetail(uint16_t x, uint16_t y, uint8_t dayIndex) {
  tft.setColor(BLUE);
  tft.setFont(6);
  ui.setTextAlignment(CENTER);
  String day = wunderground.getForecastTitle(dayIndex).substring(0, 3);
  day.toUpperCase();
  tft.setColor(WHITE);
  ui.drawString(x + 2, y + 18, day+" ");
  tft.setFont(font_System10);
  tft.setColor(BLUE);
  tft.print(wunderground.getForecastLowTemp(dayIndex) + "|" + wunderground.getForecastHighTemp(dayIndex));  
  String weatherIcon = getMeteoconIcon(wunderground.getForecastIcon(dayIndex));
  ui.drawBmp("/mini/" + weatherIcon + ".bmp", x, y + 18);
}

// draw moonphase and sunrise/set and moonrise/set
void drawAstronomy() {
  tft.setDrawWindow(0, 198, 176, 24);  
  tft.setBgColor(BLACK);
//  tft.cleanDrawWindow(); //clear draw window use the new back ground color

  int moonAgeImage = 24 * wunderground.getMoonAge().toInt() / 30.0;
  ui.drawBmp("/moon" + String(moonAgeImage) + ".bmp", 78, 0,  4);
  
  tft.setFont(6);  

  tft.setDrawWindow(0, 198, 76, 24);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow(); //clear draw window use the new back ground color
  tft.setDrawWindow(0, 198, 176, 24);  

  ui.setTextAlignment(LEFT);
  tft.setColor(BLUE);
  ui.drawString(10, 12, wunderground.getSunriseTime());
  tft.setColor(WHITE);
  tft.print(" Sun ");
  tft.setColor(BLUE);
  tft.print(wunderground.getSunsetTime());

  tft.setDrawWindow(100, 198, 76, 24);  
  tft.setBgColor(BLACK);
  tft.cleanDrawWindow(); //clear draw window use the new back ground color
  tft.setDrawWindow(0, 198, 176, 24);  

  ui.setTextAlignment(RIGHT);
  tft.setColor(BLUE);
  ui.drawString(100, 12, wunderground.getMoonriseTime());
  tft.setColor(WHITE);
  tft.print(" Moon ");
  tft.setColor(BLUE);
  tft.print(wunderground.getMoonsetTime());
  
}

// Helper function, should be part of the weather station library and should disappear soon
String getMeteoconIcon(String iconText) {
  if (iconText == "F") return "chanceflurries";
  if (iconText == "Q") return "chancerain";
  if (iconText == "W") return "chancesleet";
  if (iconText == "V") return "chancesnow";
  if (iconText == "S") return "chancetstorms";
  if (iconText == "B") return "clear";
  if (iconText == "Y") return "cloudy";
  if (iconText == "F") return "flurries";
  if (iconText == "M") return "fog";
  if (iconText == "E") return "hazy";
  if (iconText == "Y") return "mostlycloudy";
  if (iconText == "H") return "mostlysunny";
  if (iconText == "H") return "partlycloudy";
  if (iconText == "J") return "partlysunny";
  if (iconText == "W") return "sleet";
  if (iconText == "R") return "rain";
  if (iconText == "W") return "snow";
  if (iconText == "B") return "sunny";
  if (iconText == "0") return "tstorms";
  return "unknown";
}

// if you want separators, uncomment the tft-line
void drawSeparator(uint16_t y) {
   tft.drawLine(10, y, 176 - 2 * 10, y);
}

