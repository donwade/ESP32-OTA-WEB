
#include "Arduino.h"
#define LED_BUILTIN 2

#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>

#include "addin-ota.h"

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
int32_t nBrownouts;

//--------------------------------------------------------------
//https://stackoverflow.com/questions/73200166/exception-handling-for-brownout-detector-was-trigerred

#include <rom/rtc.h>

void print_reset_reason(RESET_REASON reason)
{
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
}


//interrupt_handler_t void low_voltage_save(void *notused) {

 void low_voltage_save(void *notused) 
{

    nvs_handle my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
	
    if (err != ESP_OK) {
        ets_printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
	else
	{
        err = nvs_get_i32(my_handle, "brownouts", &nBrownouts);

		if (err == ESP_ERR_NVS_NOT_FOUND)
		{
			printf("The brownout value is not initialized yet!\n");
			nBrownouts = 1;
		}
		
        err = nvs_set_i32(my_handle, "brownouts", nBrownouts);
        err = nvs_commit(my_handle);

        nvs_close(my_handle);
		
        REG_WRITE(RTC_CNTL_INT_CLR_REG, RTC_CNTL_BROWN_OUT_INT_CLR);
		
        esp_cpu_stall(!xPortGetCoreID());

        ets_printf("\r\nBrownout detector was triggered\r\n\r\n");
        //esp_restart_noos();
        
        while(1) {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
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

	nvs_handle my_handle;
	
	esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);

	if (err != ESP_OK) {
		ets_printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	} 
	else
	{
		err = nvs_get_i32(my_handle, "brownouts", &nBrownouts);

		if (err == ESP_ERR_NVS_NOT_FOUND)
		{
			printf("\nVirgin :Set Brownout to 1 !\n");
			nBrownouts = 1;
			err = nvs_set_i32(my_handle, "brownouts", nBrownouts);
			err = nvs_commit(my_handle);
		}
		
		Serial.printf("%s:%d brownout value is %d\n", __FUNCTION__, __LINE__, nBrownouts);
		nvs_close(my_handle);
	}
}


extern void web_setup(void);
extern void web_loop(void);

//void app_main()
void ota_setup()
{
    Serial.begin(115200);

    Serial.println("CPU0 reset reason: ");
    print_reset_reason(rtc_get_reset_reason(0));

    Serial.println("CPU1 reset reason: ");
    print_reset_reason(rtc_get_reset_reason(1));

    // Initialize NVS
    brownout_init();
   
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
	{
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");

    nvs_handle my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
	
    if (err != ESP_OK) 
	{
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
	else 
    {
        printf("Done\n");

        // Read
        printf("Reading restart counter from NVS ... ");
        err = nvs_get_i32(my_handle, "nResets", &nResets);
		
        switch (err) 
		{
            case ESP_OK:
                printf("Done\n");
                printf("Restart counter = %d\n", nResets);
				err = nvs_set_i32(my_handle, "nResets", nResets+1);
				Serial.printf("%s:%d err=%d\n", __FUNCTION__,__LINE__);
                break;
				
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The Restart value is not initialized yet!\n");
				
				// initialize it.
				err = nvs_set_i32(my_handle, "nResets", nResets);
				Serial.printf("%s:%d err=%d\n", __FUNCTION__,__LINE__, err);
				err = nvs_commit(my_handle);
				
                break;

            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

       // Close
        nvs_close(my_handle);
    }

  printf("\n");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  
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

	//pinMode(LED_BUILTIN, OUTPUT);  // blinky

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
