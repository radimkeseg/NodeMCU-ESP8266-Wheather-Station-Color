# NodeMCU esp8266-weather-station

NodeMCU ESP8266 Weather Station with Color Dicole Display

## Setup

* Download this project either with a git checkout or press "Download as zip"
* Install the following librarys with your Arduino Library Manager in Sketch > Include Library > Manage Libraries...
 * Adafruit_STMPE610
 * Adafruit-GFX-Library
 * esp8266-weather-station
 * json-streaming-parser
 * WiFiManager
* Go to http://wunderground.com, create an account and get an API Key
* Open the sketch in the Arduino Include and
 * Enter  the Wunderground API Key
 * Enter your Wifi credentials
 * Adjust the location according to Wunderground API, e.g. Zurich, CH
 * Adjust UTC offset
 
## Components
* universal PCB http://www.ebay.com/itm/400985368376?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT
* NodeMCU ESP8266 http://www.ebay.com/itm/401009563436?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT
* Dicole Display http://www.ebay.com/itm/371295963140?_trksid=p2060353.m2749.l2649&ssPageName=STRK%3AMEBIDX%3AIT

## Known issues
* Date is not properly reflected for non GMT zones (within the offset only)
* Occasionally, update freeze (suspect - in case of the network issue)

## Inspiration
* https://learn.adafruit.com/wifi-weather-station-with-tft-display/

## Libraries
* Libraries folder
* http://www.digole.com/images/file/Tech_Data/DigoleSerial.zip
* lib folder contains update of certain library files to make the station working properly - find them in library and replace by files from lib