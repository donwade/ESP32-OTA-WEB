#include <WiFi.h>
#include "viewController.h"
#include "watchdogs.h"
#include "TimeLib.h"
#include "addin-ota.h"

#define LINE Serial.printf("%s:%d\n", __FUNCTION__, __LINE__)

#include <M5Unified.h>
#include <SPI.h>
#include <SD.h>
#include "m5Core2-only.h"
#include "RTC.h"
#include "batmon.h"



#define MAIN_GPS_FILENAME "/xxxx.db"
#define BACkUP_GPS_FILENAME "/yyyy.db"


uint8_t copyBuffer[64];

time_t unixTime; // a time stamp

File hSrcFile;
File hDestFile;

//---------------------------------------------------------

//-------------------------------------------------------------

bool cloneFile (char *src_filename, char *dest_filename)
{
	int bytesCopied = 0;
	
	hSrcFile = SD.open(src_filename);
	if (hSrcFile)
	{
		Serial.printf("opened source file %s\n", src_filename);
	}
	else 
	{
		Serial.printf("source file not found %s\n", src_filename);
		return 0;
	}

	unsigned int startTime = millis();

	if(SD.exists(dest_filename)) SD.remove(dest_filename);

	
	hDestFile = SD.open(dest_filename, FILE_WRITE);
	
	if(hDestFile) 
	{
		
		while (hSrcFile.available()>0) 
		{
			int iBytesRead;
			
			iBytesRead = hSrcFile.read(copyBuffer, sizeof(copyBuffer));
			
			if (iBytesRead) 
			{
				bytesCopied += iBytesRead;
		 		hDestFile.write(copyBuffer, iBytesRead);
			}
		}
	}
	else
	{
		Serial.printf("can't open dest for writing %s\n", dest_filename);
		return false;
	}
		
	
	hSrcFile.close();
	hDestFile.close();
	
	Serial.printf( "DONE. %s -> %s %d bytes at %d mS\n", 
			src_filename, dest_filename,
			bytesCopied,
			millis()-startTime 
			);
	
	return true;
	
}

//-------------------------------------------------------------
#define MAIN_GPS_FILENAME "/gps.db"
#define BACkUP_GPS_FILENAME "/backup.db"

void * savingMode(BUTTON_EVENT some_key)
{
	IPAddress ip;
	batt_stats battery;
	uint8_t lineno = 1;
	
	getBatteryStats (&battery);

	cprintf(_YELLOW, lineno++, "%3.1fv %3d%% %4dmA %s", 
						battery.batt_mV/1000.,
						battery.percent,
						battery.current_mA,
						battery.chargeDirection < 0 ? "DIS" : 
						battery.chargeDirection > 0 ? "CHG" : "STP");


	cprintf(_GREEN, lineno++, "host= %s", LOCAL_HOSTNAME);

/*		
	ip = WiFi.localIP();
	uint32_t x = (uint32_t) ip;
	
	cprintf(_GREEN, lineno++, "ip = %d:%d:%d:%d", 
		 x     & 0xFF,
		(x>>8 )&0xFF, 
		(x>>16)&0xFF, 
		(x>>24)&0xFF);
*/
	
	// print the received signal strength:
	cprintf(_GREEN, lineno++, "%d dBm", WiFi.RSSI());
	//cprintf(_CYAN, lineno++, "up = %d S", uptime());
	
	battmon data;  
	getBatmon(&data);
	
	//cprintf(_CYAN, lineno++, "up=%d dn=%d", data.chargeTime, data.dischargeTime);

	char chg[80];
	char dis[80];
	char hld[80];
	secondsToHMS(data.chargeTime, chg);
    secondsToHMS(data.dischargeTime, dis);
    secondsToHMS(data.holdchargeTime, hld);
	cprintf(_CYAN, lineno++, "up=%s", chg);
	cprintf(_CYAN, lineno++, "dn=%s", dis);
	cprintf(_CYAN, lineno++, "hd=%s", hld);
	
	// all display updates done ... just keys left
	if (some_key == DISPLAY_REFRESH) return (void*) savingMode;

	switch (some_key)
	{
		case BUTTON_INIT:
			lfillRect(0,0, 50, 50, _RED);
			threeButtonText("SAVE", "UNDO", "RESTORE");
			
			cprintf(_RED,	2, "TODO        ");
			cprintf(_RED ,	3, "TODO        ");
			cprintf(_GREEN, 4, "TODO        ");
			cprintf(_GREEN, 5, "TODO        ");
			cprintf(_ORANGE,6, "TODO        ");
			cprintf(_ORANGE,7, "TODO        ");

			Tdelay(2000);
			return (void *) savingMode;
			
		break;	

		case LBUTTON_UP:
		case LBUTTON_DN:
			if (some_key == LBUTTON_DN)
			{
				colourBarX(_BLUE, 99); 
				
				//copyCameraListToSD(MAIN_GPS_FILENAME);
				//cloneFile(MAIN_GPS_FILENAME, BACkUP_GPS_FILENAME);
				Tdelay(2000);
				return (void *) savingMode;
			}

		break;

		case RBUTTON_UP:
		case RBUTTON_DN:

			if (some_key == RBUTTON_DN)
			{

				//cloneFile(BACkUP_GPS_FILENAME, MAIN_GPS_FILENAME);

				cprintf(_ORANGE, 2, "watchdog to reset");
				cprintf(_ORANGE, 3, "watchdog to reset");
				cprintf(_ORANGE, 4, "watchdog to reset");
				cprintf(_ORANGE, 5, "watchdog to reset");
				cprintf(_ORANGE, 6, "watchdog to reset");
				colourBarX(_YELLOW, 99); 
				
				delay(6000); // let the dog take me out
				return (void *) savingMode;
			}
		break;

		case MBUTTON_DN:
		case MBUTTON_UP:

			if (some_key == MBUTTON_DN)
			{
				cprintf(_ORANGE, 7, "last add removed");
				colourBarX(_RED, 50); 

				Tdelay(2000);
				return (void *) savingMode;
			}			
			
		break;

	}

	return (void *) savingMode;
}

//--------------------------------------------------------------

void runDisplayTask(void *not_used)
{
	static bool bFirstTime = true;
	BUTTON_EVENT abutton;
	
	if (bFirstTime) 
	{
		bFirstTime = false;
		button_push(BUTTON_INIT);
	}

 	while (true)
	{
		abutton = button_pop(250);
		savingMode(abutton);
	}
}

