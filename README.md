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

## Known issues
* Date is not properly reflected for non GMT zones (within the offset only)
* Occasionally the update freeze (suspect - in case of the network issue)

## Inspiration
* https://learn.adafruit.com/wifi-weather-station-with-tft-display/
