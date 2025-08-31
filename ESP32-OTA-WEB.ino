/*


  this example will show
  1. how to use and ESP 32 for reading pins
  2. building a web page for a client (web browser, smartphone, smartTV) to connect to
  3. sending data from the ESP to the client to update JUST changed data
  4. sending data from the web page (like a slider or button press) to the ESP to tell the ESP to do something

  If you are not familiar with HTML, CSS page styling, and javascript, be patient, these code platforms are
  not intuitive and syntax is very inconsitent between platforms

  I know of 4 ways to update a web page
  1. send the whole page--very slow updates, causes ugly page redraws and is what you see in most examples
  2. send XML data to the web page that will update just the changed data--fast updates but older method
  3. JSON strings which are similar to XML but newer method
  4. web sockets very very fast updates, but not sure all the library support is available for ESP's

  I use XML here...

  compile options
  1. esp32 dev module
  2. upload speed 921600
  3. cpu speed 240 mhz
  flash speed 80 mhz
  flash mode qio
  flash size 4mb
  partition scheme default


  NOTE if your ESP fails to program press the BOOT button during programm when the IDE is "looking for the ESP"

  The MIT License (MIT)

  code writen by Kris Kasprzak
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  On a personal note, if you develop an application or product using this code 
  and make millions of dollars, I'm happy for you!

*/

#include <WiFi.h>       // standard library
#include <WebServer.h>  // standard library
#include "html-webpage.h"   // .h file that stores your html page code

#include <M5Unified.h>
#include <TinyGPS++.h>
#include <SD.h>

#include "m5Core2-only.h"
#include "viewController.h"
#include "watchdogs.h"
#include "viewController.h"
#include "rtc_wdt.h"
#include "esp_debug_helpers.h"
#include "RTC.h"
#include "batmon.h"

void runDisplayTask(void *not_used);
void runPingTask(void *not_used);


// here you post web pages to your homes intranet which will make page debugging easier
// as you just need to refresh the browser as opposed to reconnection to the web server

#define USE_LOCAL_ROUTER

// replace this with your homes intranet connect parameters
#define LOCAL_SSID "your_home_ssid"
#define LOCAL_PASS "your_home_passwrord"

#ifndef USE_LOCAL_ROUTER
// make your own hotspot, provide SSID and PASSWORD
#define AP_SSID "TestWebSite"
#define AP_PASS "023456789"
#endif

#if 0
// start your defines for pins for sensors, outputs etc.
#define PIN_ARBITRARY_OUTPUT 26 // connected to nothing but an example of a digital write from the web page
#define PIN_FAN_PMW 		 27 // pin 27 and is a PWM signal to control a fan speed
#define PIN_LED 			  2 //On board LED
#define P32_WHT_RDR			 34 // some analog input sensor
#define P33_YLW_NC 			 35 // some analog input sensor
#else
// start your defines for pins for sensors, outputs etc.
#define PIN_ARBITRARY_OUTPUT -1 // connected to nothing but an example of a digital write from the web page
#define PIN_FAN_PMW 		 -1 // pin 27 and is a PWM signal to control a fan speed
#define PIN_LED 			 -1 //On board LED
#define P32_WHT_RDR			 32 // 
#endif

// variables to store measure data and sensor states
int		A2D_P32 = 0, 	A2D_P33 = 0;
float 	A2D_P32_mV = 0, A2D_P33_mV = 0;

bool LED0 = false, SomeOutput = false;

uint32_t lastSensorTime = 0;

int FanSpeed = 0;
int FanRPM = 0;

// the XML array size needs to be bigger that your maximum expected size. 2048 is way too big for this example
char XML[2048];

// just some buffer holder for char operations
char xml_tbuf[320];

// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress Actual_IP;

// definitions of your desired intranet created by the ESP32
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

// gotta create a server
WebServer server(80);


extern void ota_setup(void);
extern void ota_loop(void);

static const gpio_num_t SDCARD_CSPIN = GPIO_NUM_4;

void setup() {

  setup_M5();
  Serial.printf("BUILT ON %s %s *** ESP-IDF VER = %s ***\n", __DATE__, __TIME__, esp_get_idf_version());                                                                          

  bool ok = SD.begin(SDCARD_CSPIN, SPI, 8000000); 
  Serial.printf("SD=%d\n", ok); 
  lsetTextColor(_WHITE, _BLACK);
  lsetCursor(0, 0);
  
  ota_setup();
  
  //pinMode(PIN_FAN_PMW, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  pinMode(P32_WHT_RDR, INPUT); //was analogRead(P32_WHT_RDR);

  // turn off led
  LED0 = false;
  digitalWrite(PIN_LED, LED0);

  // configure LED PWM functionalitites
  /* old ESP compiler
  ledcSetup(0, 10000, 8);
  ledcAttachPin(PIN_FAN_PMW, 0);
  ledcWrite(0, FanSpeed);
  */

  // if your web page or XML are large, you may not get a call back from the web page
  // and the ESP will think something has locked up and reboot the ESP
  // you may consider disabling the watch dog timers
  // disableCore0WDT();
  // disableCore1WDT();

  // just an update to progress


#if 0  // DONE IN add-in.ota
  // standard stuff here
  Serial.begin(9600);

  // if you have this #define USE_LOCAL_ROUTER,  you will connect to your home intranet, again makes debugging easier
#ifdef USE_LOCAL_ROUTER
  WiFi.begin(LOCAL_SSID, LOCAL_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  Actual_IP = WiFi.localIP();
#endif
#endif

  // if you don't have #define USE_LOCAL_ROUTER, here's where you will creat and access point
  // an intranet with no internet connection. But Clients can connect to your intranet and see
  // the web page you are about to serve up
  
#ifndef USE_LOCAL_ROUTER
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: "); Serial.println(Actual_IP);
#endif

  Serial.println("starting server");
  printWifiStatus();


  // these calls will handle data coming back from your web page
  // this one is a page request, upon ESP getting / string the web page will be sent
  server.on("/", SendWebsite);

  // upon esp getting /XML string, ESP will build and send the XML, this is how we refresh
  // just parts of the web page
  
  server.on("/xml", SendXML);

  // upon ESP getting /UPDATE_SLIDER string,
  // ESP will execute the UpdateSlider function
  // same notion for the following .on calls
  // add as many as you need to process incoming strings from your web page
  // as you can imagine you will need to code some javascript in your web page to send such strings
  // this process will be documented in the html-webpage.h web page code
  
  server.on("/UPDATE_SLIDER", UpdateSlider);
  server.on("/BUTTON_0", UserPressLEDbutton);
  server.on("/BUTTON_1", UserPressSwitchButton);

  // finally begin the server
  server.begin();

  spawnTaskAndDogV2( runLightBarTask, //(void * not_used)TaskFunction_t pvTaskCode,
                     "LightBarTask",  //const char * const pcName,
                     1024 * 8,        //const uint32_t usStackDepth,
                     NULL,            //void * const pvParameters,
                     4                //UBaseType_t uxPriority)
                     );

  spawnTaskAndDogV2( runDisplayTask, //(void * not_used)TaskFunction_t pvTaskCode,
                     "DisplayTask",  //const char * const pcName,
                     1024 * 8,        //const uint32_t usStackDepth,
                     NULL,            //void * const pvParameters,
                     4                //UBaseType_t uxPriority)
                     );


spawnTaskAndDogV2( runPingTask, 	//(void * not_used)TaskFunction_t pvTaskCode,
				   "PingTask",  	//const char * const pcName,
				   1024 * 8,		//const uint32_t usStackDepth,
				   NULL,			//void * const pvParameters,
				   4				//UBaseType_t uxPriority)
				   );

#if 1
spawnTaskAndDogV2( runBatmonTask, 	//(void * not_used)TaskFunction_t pvTaskCode,
				   "BatmonTask",  	//const char * const pcName,
				   1024 * 4,		//const uint32_t usStackDepth,
				   NULL,			//void * const pvParameters,
				   4				//UBaseType_t uxPriority)
				   );
#endif
}

void loop() {

  static int8_t p32 = -1;
  static int8_t p33 = -1;
  
  ota_loop();
  
  // you main loop that measures, processes, runs code, etc.
  // note that handling the "on" strings from the web page are NOT in the loop
  // that processing is in individual functions all managed by the wifi lib

  // in my example here every 50 ms, i measure some analog sensor data (my finger dragging over the pins
  // and process accordingly
  // analog input can be from temperature sensors, light sensors, digital pin sensors, etc.
  
  if ((millis() - lastSensorTime) >= 50) 
  {
    //Serial.println("Reading Sensors");
    lastSensorTime = millis();
    A2D_P32 =  digitalRead(P32_WHT_RDR); //analogRead(P32_WHT_RDR);

	if (p32 != A2D_P32)
	{
		p32 = A2D_P32;
		Serial.printf("ssssssssssssss p32 = %d\n", p32);
		p32 ? setToggleColors(_RED, _BLUE, 2) :
			  setToggleColors(_BLACK, _BLACK, 2);
	}

	if (p33 != A2D_P33)
	{
		p33 = A2D_P33;
		Serial.printf("ssssssssssssss p33 = %\n", p33);
	}


    // standard converion to go from 12 bit resolution reads to volts on an ESP
    A2D_P32_mV = A2D_P32 * 3300;
    A2D_P33_mV = A2D_P33 * 3300;

    // standard converion to go from 12 bit resolution reads to volts on an ESP
    //A2D_P32_mV = A2D_P32 * 3300 / 4096;
    //A2D_P33_mV = A2D_P33 * 3300 / 4096;

  }

  // no matter what you must call this handleClient repeatidly--otherwise the web page
  // will not get instructions to do something
  server.handleClient();
  	
}


// function managed by an .on method to handle slider actions on the web page

// this example will get the passed string called VALUE and conver to a pwm value
// and control the fan speed
void UpdateSlider() {

  // man I hate strings, but wifi lib uses them...
  String t_state = server.arg("VALUE");

  // conver the string sent from the web page to an int
  FanSpeed = t_state.toInt();
  
  Serial.print("UpdateSlider"); Serial.println(FanSpeed);
  // now set the PWM duty cycle
  
  // old ESP compiler
  // ledcWrite(0, FanSpeed);

  // latest ESP compiler
  analogWrite(PIN_FAN_PMW, FanSpeed); // config for PMW out mode


  // YOU MUST SEND SOMETHING BACK TO THE WEB PAGE--BASICALLY TO KEEP IT LIVE

  // option 1: send no information back, but at least keep the page live
  // just send nothing back
  // server.send(200, "text/plain", ""); //Send web page

  // option 2: send something back immediately, maybe a pass/fail indication, maybe a measured value
  // here is how you send data back immediately and NOT through the general XML page update code

  // my simple example guesses at fan speed--ideally measure it and send back real data
  // i avoid strings at all caost, hence all the code to start with "" in the buffer and build a
  // simple piece of data

  // slider goes from 0 - 255, rpm goes from 0 - 2400
  
  FanRPM = map(FanSpeed, 0, 255, 0, 2400);
  
  strcpy(xml_tbuf, "");
  sprintf(xml_tbuf, "%d", FanRPM);
  
  // now send rpm  back to webpage for display
  server.send(200, "text/plain", xml_tbuf); //Send web page

}

// now process LED on/off press from the web site. Typical applications are the used on the web client can
// turn on / off a light, a fan, disable something etc

void UserPressLEDbutton() 
{

  LED0 = !LED0;
  
  digitalWrite(PIN_LED, LED0);
  Serial.print("Button 0 "); Serial.println(LED0);
  
  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or let the XML manage
  // sending feeback

  // option 1 -- keep page live but dont send any thing
  // here i don't need to send and immediate status, any status
  // like the illumination status will be send in the main XML page update
  // code
  server.send(200, "text/plain", ""); //Send web page

  // option 2 -- keep page live AND send a status
  // if you want to send feed back immediataly
  // note you must have reading code in the java script
  /*
    if (LED0) {
    server.send(200, "text/plain", "1"); //Send web page
    }
    else {
    server.send(200, "text/plain", "0"); //Send web page
    }
  */

}

// same notion for processing button_1
void UserPressSwitchButton() {

  // just a simple way to toggle a THINGY on/off. Much better ways to do this
  
  Serial.println("Button 1 press");
  SomeOutput = !SomeOutput;

  digitalWrite(PIN_ARBITRARY_OUTPUT, SomeOutput);
  Serial.print("Button 1 "); Serial.println(LED0);
  
  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or send all data via XML use this method
  // sending feeback

  server.send(200, "text/plain", ""); //Send web page

  // if you want to send feed back immediataly
  // note you must have proper code in the java script to read this data stream
  /*
    if (some_process) {
    server.send(200, "text/plain", "SUCCESS"); //Send web page
    }
    else {
    server.send(200, "text/plain", "FAIL"); //Send web page
    }
  */
}


// code to send the main web page
// PAGE_MAIN is a large char defined in html-webpage.h
void SendWebsite() {

  Serial.println("sending web page");

  server.send(200, "text/html", PAGE_MAIN);

}

// code to send the main web page
// I avoid string data types at all cost hence all the char mainipulation code
void SendXML() {

  Serial.printf("%s:%d %s\n", __FUNCTION__, __LINE__, format_date_time());

  // Serial.println("sending xml");

  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  sprintf(xml_tbuf, "<RADAR1>%d</RADAR1>\n", A2D_P32);
  strcat(XML, xml_tbuf);
  
  sprintf(xml_tbuf, "<RADAR2>%d</RADAR2>\n", (int) (A2D_P32_mV));
  strcat(XML, xml_tbuf);

  batt_stats batman;
  getBatteryStats (&batman);

  // send battery voltage
  sprintf(xml_tbuf, "<BATVOLTAGE1>%5.2fv</BATVOLTAGE1>\n", batman.volt_mV/1000.);
  strcat(XML, xml_tbuf);

  sprintf(xml_tbuf, "<BATVOLTAGE2>%d%% </BATVOLTAGE2>\n", batman.percent);
  strcat(XML, xml_tbuf);


  sprintf(xml_tbuf, "<BATCURRENT1>%d</BATCURRENT1>\n", batman.current_mA);
  strcat(XML, xml_tbuf);

  sprintf(xml_tbuf, "<BATCURRENT2>%s</BATCURRENT2>\n", batman.chargeDirection ? "CHARGE" : "DISCHARGE");
  strcat(XML, xml_tbuf);

  sprintf(xml_tbuf, "<UPTIME1>%d</UPTIME1>\n", uptime());
  strcat(XML, xml_tbuf);

  sprintf(xml_tbuf, "<REBOOTS1>%d</REBOOTS1>\n", 5);
  strcat(XML, xml_tbuf);

  sprintf(xml_tbuf, "<REBOOTS2>%d</REBOOTS2>\n", 69);
  strcat(XML, xml_tbuf);


  // show led0 status
  if (LED0) {
    strcat(XML, "<LED>1</LED>\n");
  }
  else {
    strcat(XML, "<LED>0</LED>\n");
  }

  if (SomeOutput) {
    strcat(XML, "<SWITCH>1</SWITCH>\n");
  }
  else {
    strcat(XML, "<SWITCH>0</SWITCH>\n");
  }

  strcat(XML, "</Data>\n");

  // wanna see what the XML code looks like?
  // actually print it to the serial monitor and use some text editor to get the size
  // then pad and adjust char XML[2048]; above

  Serial.println(XML);

  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms

  server.send(200, "text/xml", XML);


}

// I think I got this code from the wifi example
void printWifiStatus() {

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("Open http://");
  Serial.println(ip);
}

// end of code
