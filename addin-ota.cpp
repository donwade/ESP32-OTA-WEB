
#include "Arduino.h"
#define LED_BUILTIN 2

#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>

#include "watchdogs.h"
#include "addin-ota.h"
#include "RTC.h"
#include "nv.h"

#include "common.h"

//#include "esp_brownout_detector.h" // Include the brownout detector header

#ifndef MY_SSID
const char *ssid = "wifi";
const char *password =   "";
#error WTF
#else
const char *ssid = MY_SSID;
const char *password = MY_SSID_PASSWORD;
#endif

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "soc/soc.h"
//#include "soc/cpu.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_cntl.h"

#ifdef CONFIG_BROWNOUT_DET_LVL
#define BROWNOUT_DET_LVL CONFIG_BROWNOUT_DET_LVL
#else
#define BROWNOUT_DET_LVL 5
#endif //CONFIG_BROWNOUT_DET_LVL

#define CONFIG_BROWNOUT_DET_LVL_SEL_5 1

int32_t nResets;
int32_t nBrownoutCtr;

//--------------------------------------------------------------
//https://stackoverflow.com/questions/73200166/exception-handling-for-brownout-detector-was-trigerred

#include <rom/rtc.h>

uint8_t getResetReason(char *msg, RESET_REASON reason)
{
  Serial.printf("\n%s ", msg);

  if (reason < 10 && reason > 6)
  {
    int32_t value = 0; // default
  	nvIncrementValue("DOG_CTR", &value);
	TRACE ("****** WATCHDOG RESET FOUND\n");
  }
  
  switch (reason)
  {
    /**<1, Vbat power on reset*/
    case 1 : Serial.println ("POWERON_RESET");break;          
    
    /**<3, Software reset digital core*/
    case 3 : Serial.println ("SW_RESET");break;               
    
    /**<4, Legacy watch dog reset digital core*/
    case 4 : Serial.println ("OWDT_RESET");break;             
    
    /**<5, Deep Sleep reset digital core*/
    case 5 : Serial.println ("DEEPSLEEP_RESET");break;        
    
    /**<6, Reset by SLC module, reset digital core*/
    case 6 : Serial.println ("SDIO_RESET");break;             
    
    /**<7, Timer Group0 Watch dog reset digital core*/
    case 7 : Serial.println ("TG0WDT_SYS_RESET");break;       
    
    /**<8, Timer Group1 Watch dog reset digital core*/
    case 8 : Serial.println ("TG1WDT_SYS_RESET");break;       
    
    /**<9, RTC Watch dog Reset digital core*/
    case 9 : Serial.println ("RTCWDT_SYS_RESET");break;       
    
    /**<10, Instrusion tested to reset CPU*/
    case 10 : Serial.println ("INTRUSION_RESET");break;       
    
    /**<11, Time Group reset CPU*/
    case 11 : Serial.println ("TGWDT_CPU_RESET");break;       
    
    /**<12, Software reset CPU*/
    case 12 : Serial.println ("SW_CPU_RESET");break;          
    
    /**<13, RTC Watch dog Reset CPU*/
    case 13 : Serial.println ("RTCWDT_CPU_RESET");break;      
    
    /**<14, for APP CPU, reseted by PRO CPU*/
    case 14 : Serial.println ("EXT_CPU_RESET");break;         
    
    /**<15, Reset when the vdd voltage is not stable*/
    case 15 : Serial.println ("RTCWDT_BROWN_OUT_RESET");break;
    
    /**<16, RTC Watch dog reset digital core and rtc module*/
    case 16 : Serial.println ("RTCWDT_RTC_RESET");break;      
    
    default : Serial.println ("NO_MEAN");
  }
  
  Serial.println();
  return reason;
}


//interrupt_handler_t void low_voltage_save(void *notused) {

 void low_voltage_save(void *notused) 
{
	int32_t nBrownoutCtr = 0;
	
	bool pass = nvIncrementValue("BROWNOUT_CTR", &nBrownoutCtr);
	if (!pass)
	{
		TRACE("\nVirgin :Set Brownout to 1 !\n");
		nBrownoutCtr = 1;
		nvSetValue("BROWNOUT_CTR", nBrownoutCtr);
	}
	
	TRACE("brownout value is %d\n", nBrownoutCtr);


	// stop brown interrupts.
	REG_WRITE(RTC_CNTL_INT_CLR_REG, RTC_CNTL_BROWN_OUT_INT_CLR);

	//halt the cpu. Stop any damage.
	esp_cpu_stall(!xPortGetCoreID());

	ets_printf("\r\nBrownout detector was triggered\r\n\r\n");
	//esp_restart_noos();

	while(1)
	{
	    vTaskDelay(1 / portTICK_PERIOD_MS);
	}

}



/*esp_err_t rtc_isr_register(intr_handler_t handler,
						  	 void* handler_arg,
                             uint32_t rtc_intr_mask, // set for brown out 
                             uint32_t flags);		 // =0, don't care about caching
*/

void brownout_init()
{

    REG_WRITE(RTC_CNTL_BROWN_OUT_REG,
            RTC_CNTL_BROWN_OUT_ENA /* Enable BOD */
            | RTC_CNTL_BROWN_OUT_PD_RF_ENA /* Automatically power down RF */
            /* Reset timeout must be set to >1 even if BOR feature is not used */
            | (2 << RTC_CNTL_BROWN_OUT_RST_WAIT_S)
            | (BROWNOUT_DET_LVL << RTC_CNTL_DBROWN_OUT_THRES_S));

    ESP_ERROR_CHECK( rtc_isr_register(low_voltage_save, 
                     				  NULL, 						//handler_arg
                                      RTC_CNTL_BROWN_OUT_INT_ENA_M, //rtc_intr_mask
                     				  0	)							//uint32_t flags
                    );

    printf("Initialized BOD\n");

    REG_SET_BIT( RTC_CNTL_INT_ENA_REG, 
				 RTC_CNTL_BROWN_OUT_INT_ENA_M);

	int32_t nBrownoutCtr = 0;
	nvGetValue("BROWNOUT_CTR", &nBrownoutCtr);
	TRACE("brownout value is %d\n", nBrownoutCtr);
}


extern void web_setup(void);
extern void web_loop(void);

//-------------------------------------------------------------
//void app_main()
void ota_setup()
{

	int32_t val;

	// Initialize NVS
	brownout_init();

	init_NVram();

	val = 0;
	nvIncrementValue("RESET_CTR", &val);
	TRACE("reset count = %d\n", val);

	val = getResetReason("CPU0: ", rtc_get_reset_reason(0));
	nvSetValue("CPU0_RESET", val);
	nvGetValue("CPU0_RESET", &val);

	val = getResetReason("CPU1: ", rtc_get_reset_reason(1));
	nvSetValue("CPU1_RESET", val);
	nvGetValue("CPU1_RESET", &val);

/*
  // https://deepbluembedded.com/esp32-change-cpu-speed-clock-frequency/

  // normal values.
  TRACE ota_setup:460   Xtal frequency = 40 mHz 								  
  TRACE ota_setup:461   CpuFrequency = 240 mHz
  TRACE ota_setup:462   APB bus frequency = 80 mHz
  
  // moving clock to CPU clock to 40 
  //  look at XTAL table below, find the XTAL we have (its 40)
  //  find the divider value (its 10 for us)
  //function takes the following frequencies as valid values:
  //  (240 160) = 80  <<< For all XTAL types
  //  ( 40  20) = 10  <<< For 40MHz XTAL
  //  ( 26)     = 13  <<< For 26MHz XTAL
  //  ( 24)     = 12  <<< For 24MHz XTAL
*/


	setCpuFrequencyMhz(CPU_FREQ);  // 10 thru 40 ... fails on wifi

	Serial.println();
	TRACE ("-------- CPU FREQ %d/240 -------------------\n", CPU_FREQ);
	TRACE ("Xtal frequency = %d mHz (normally 40)\n", getXtalFrequencyMhz());
	TRACE ("CpuFrequency = %d mHz (normally 240)\n", getCpuFrequencyMhz());
	TRACE ("APB bus frequency = %d mHz (normally 80)\n",  getApbFrequency()/1000000);


	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.waitForConnectResult() != WL_CONNECTED) 
	{
		Serial.println("Connection Failed! Rebooting...");
		delay(5000); 
		ESP.restart();
	}

	#if 0
		TRACE("initRTCfromNTP NOT being called. Coin testing\n");
	#else
		initRTCfromNTP();
	#endif

	// Port defaults to 3232
	ArduinoOTA.setPort(3232);

	// Hostname defaults to esp3232-[MAC]
	ArduinoOTA.setHostname(LOCAL_HOSTNAME);
	Serial.printf("remote hostname = %s.local\n", LOCAL_HOSTNAME);
	
	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA
	.onStart([]() {
	  String type;
	  if (ArduinoOTA.getCommand() == U_FLASH) {
	    type = "sketch";
	  } else {  // U_SPIFFS
	    type = "filesystem";
	  }

	  // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
	  Serial.println("Start updating " + type);
	})
	.onEnd([]() {
	  Serial.println("\nEnd");
	})
	.onProgress([](unsigned int progress, unsigned int total) {
	  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	})
	.onError([](ota_error_t error) {
	  Serial.printf("Error[%u]: ", error);
	  if (error == OTA_AUTH_ERROR) {
	    Serial.println("Auth Failed");
	  } else if (error == OTA_BEGIN_ERROR) {
	    Serial.println("Begin Failed");
	  } else if (error == OTA_CONNECT_ERROR) {
	    Serial.println("Connect Failed");
	  } else if (error == OTA_RECEIVE_ERROR) {
	    Serial.println("Receive Failed");
	  } else if (error == OTA_END_ERROR) {
	    Serial.println("End Failed");
	  }
	});



	ArduinoOTA.begin();

	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

}

void ota_loop() 
{

#if 0
	// blinky
	static uint32_t ticker;
	static bool val;

	uint32_t diff = millis() - ticker;
	if (diff > 250)
	{
		digitalWrite(LED_BUILTIN,val);
		val = !val;
		ticker = millis();
	}
#endif

	ArduinoOTA.handle();
}

//-------------------------------------------------------------

void reportWakeUpCause()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {

    case ESP_SLEEP_WAKEUP_UNDEFINED:
		Serial.println(	"Wakeup:In case of deep sleep, reset was not:  exit from deep sleep");
	break;
		
    case ESP_SLEEP_WAKEUP_ALL:
		Serial.println("Wakeup:Not a wakeup cause, used to disable all wakeup sources with esp_sleep_disable_wakeup_source");
	break;
		
    case ESP_SLEEP_WAKEUP_EXT0:
		Serial.println("Wakeup:external signal using RTC_IO");
	break;
		
    case ESP_SLEEP_WAKEUP_EXT1:
		Serial.println("Wakeup:external signal using RTC_CNTL");
	break;
		
    case ESP_SLEEP_WAKEUP_TIMER:
		Serial.println("Wakeup:timer");
	break;
		
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
		Serial.println("Wakeup:touchpad");
	break;
		
    case ESP_SLEEP_WAKEUP_ULP:
		Serial.println("Wakeup:ULP program");
	break;
		
    case ESP_SLEEP_WAKEUP_GPIO:
		Serial.println("Wakeup:GPIO (light sleep only on ESP32, S2 and S3)");
	break;
		
    case ESP_SLEEP_WAKEUP_UART:
		Serial.println("Wakeup:UART (light sleep only)");
	break;
		
    case ESP_SLEEP_WAKEUP_WIFI:
		Serial.println("Wakeup:WIFI (light sleep only)");
	break;
		
    case ESP_SLEEP_WAKEUP_COCPU:
		Serial.println("Wakeup:COCPU");
	break;
		
    case ESP_SLEEP_WAKEUP_COCPU_TRAP_TRIG:
		Serial.println("Wakeup:COCPU crash");
	break;
		
    case ESP_SLEEP_WAKEUP_BT:
		Serial.println("Wakeup:BT (light sleep only)");
	break;
		
    case ESP_SLEEP_WAKEUP_VAD:
		Serial.println("Wakeup:VAD");
	break;

    default : 
		Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason);
	break;
  }
}

//-------------------------------------------------------------
// home/dwade/.arduino15/packages/m5stack/tools
// /esp32-arduino-libs/idf-release_v5.4-858a988d-v1
// /esp32s3/include/esp_hw_support/include/esp_sleep.h

#include "esp_sleep.h"
#include "driver/gpio.h"

uint8_t doNothing;

// ISR for wakeupPin
void IRAM_ATTR handleInterrupt1() 
{
    doNothing++;  // stop optimizer from deleting entire function.
}

void setupLightSleepByTimer(uint32_t timeMs)
{
	esp_sleep_enable_timer_wakeup(timeMs * 1000);
	TRACE("prepared light sleep timer for %d mS\n", timeMs);
}

void enterLightSleepTimer(void)
{
	uint32_t now = getUTCfromRTC();
	esp_light_sleep_start();
	TRACE("exit sleep.. time = %d000 mS\n", getUTCfromRTC() - now);
}

void setupSleepByGPIO(gpio_num_t wakeupPin) 
{
    //pinMode(wakeupPin, INPUT_PULLUP); // pull-up resistor

    // Configure GPIOs as wake-up source
    gpio_wakeup_enable(wakeupPin, GPIO_INTR_LOW_LEVEL); // Trigger wake-up on high level

    // Enable GPIO wake-up source
    esp_err_t result = esp_sleep_enable_gpio_wakeup();

    if (result == ESP_OK) {
        Serial.println("GPIO Wake-Up set successfully.");
    } else {
        Serial.println("Failed to set GPIO Wake-Up as wake-up source.");
    }

    // Attach interrupts to GPIO pin
    attachInterrupt(digitalPinToInterrupt(wakeupPin), 
    				handleInterrupt1, 
    				FALLING);
	
}

//-------------------------------------------------------------
void enterLightSleepGPIO() 
{
    TRACE("Enter Sleep\n");
	uint32_t ms = millis();
    esp_light_sleep_start();    // Enter light sleep
    Serial.printf("  Woke up at %d ms\n", millis() - ms);
	
    reportWakeUpCause();
}


