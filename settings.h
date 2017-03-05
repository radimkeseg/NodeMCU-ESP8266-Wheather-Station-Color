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

// Setup
const int UPDATE_INTERVAL_SECS = 10 * 60;  // Update every 10 minutes
boolean USE_TOUCHSCREEN_WAKE = false;       // use the touchscreen to wake up, ~90mA current draw
boolean DEEP_SLEEP = false;                 // use the touchscreen for deep sleep, ~10mA current draw but doesnt work
int     AWAKE_TIME = 5;                   // how many seconds to stay 'awake' before going back to zzz

// Pins for the DIGOLE display
#define TFT_I2C_DATA D1
#define TFT_I2C_CLOCK D2
#define TFT_POWER_PIN D3

// pins for the touchscreen
#define STMPE_CS 16
#define STMPE_IRQ 4

// TimeClient settings
static float UTC_OFFSET = 1;

// Wunderground Settings
const boolean IS_METRIC = true;
const String WUNDERGRROUND_API_KEY = "809a5d6ae49b49ab";
static String WUNDERGRROUND_LANGUAGE = "EN";
static String WUNDERGROUND_COUNTRY = "Czech_Republic";
static String WUNDERGROUND_CITY = "Tisnov";

const String CUSTOM_SETTINGS = "/settings/custom.txt";

// List, so that the downloader knows what to fetch
String wundergroundIcons [] = {"chanceflurries","chancerain","chancesleet","chancesnow","clear","cloudy","flurries","fog","hazy","mostlycloudy","mostlysunny","partlycloudy","partlysunny","rain","sleet","snow","sunny","tstorms","unknown"};

/***************************
 * End Settings
 **************************/

