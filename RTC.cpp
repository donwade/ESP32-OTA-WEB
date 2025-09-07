#include <M5Unified.h>
#include <TimeLib.h>
#include "RTC.h"
#include "common.h"

//-----------------------------------------------------------------
char *formattedTimeRTC() 
{ //easy way to print date and time

	static char tstring[40]; 

	m5::rtc_time_t TimeStruct;
	m5::rtc_date_t DateStruct;
	
	M5.Rtc.getTime(&TimeStruct);
	M5.Rtc.getDate(&DateStruct);

	sprintf(tstring, "%02d/%02d/%4d %02d:%02d:%02d", 
			DateStruct.date,
			DateStruct.month, 
			DateStruct.year, 
			TimeStruct.hours, 
			TimeStruct.minutes, 
			TimeStruct.seconds);
	
	return tstring; 
}
//-----------------------------------------------------------------

uint32_t getUTCfromRTC(void)
{

	time_t UTC; 		// a time stamp
	
	// cold read the RTC. Did the time stick?
	m5::rtc_time_t TimeStruct;
	m5::rtc_date_t DateStruct;
	
	tmElements_t tmpTime;	//Time elements structure
	
	M5.Rtc.getTime(&TimeStruct);
	M5.Rtc.getDate(&DateStruct);

	/*
	Serial.printf("COLD READ RTC time %02d:%02d:%02d\n",
	TimeStruct.hours,
	TimeStruct.minutes,
	TimeStruct.seconds);
	
	Serial.printf("COLD READ RTC date %02d/%02d/%04d\n",
	DateStruct.date,
	DateStruct.month,
	DateStruct.year);
	*/

	tmpTime.Hour = TimeStruct.hours;
	tmpTime.Minute = TimeStruct.minutes;
	tmpTime.Second = TimeStruct.seconds;
	
	tmpTime.Day = DateStruct.date;
	tmpTime.Month = DateStruct.month;
	
	// time starts from 1970 ... aka year 0
	tmpTime.Year = DateStruct.year - 1970;

	//thank god I dont have to calc seconds in month 
	UTC =  makeTime(tmpTime);
	
	//Serial.printf("calculated UTC from RTC = %d\n", UTC);
	return UTC;
}

//---------------------------------------------------------------------------

void setRTC(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t year)
{
		// set RTC		
		m5::rtc_time_t TimeStruct;
		m5::rtc_date_t DateStruct;
		//Serial.println("After TZ tweak");
		//Serial.print("now() = ");
		//Serial.println(now());
		Serial.println(formattedTimeRTC());

		// stuff into RTC chip
		TimeStruct.hours = hr;
		TimeStruct.minutes = min;
		TimeStruct.seconds = sec;
		
		DateStruct.year = year;
		DateStruct.month = month;
		DateStruct.date = day;
		
		M5.Rtc.setTime(&TimeStruct);
		M5.Rtc.setDate(&DateStruct);
}

//-------------------------------------------------------------
// stopwatches. can't use system tick stuff as that freezes when
// we go to sleep.
// ask the rtc for the time as it always runs.

void stpWatchStart (stopwatch *who)
{
	assert(who);
	who->started = getUTCfromRTC();
};

time_t stpWatchGetElapsedTime (stopwatch *who)
{
	assert(who);
	assert(who->started); //never started
	who->requested = getUTCfromRTC();
	return who->requested - who->started;
}

char *secondsToHMS (time_t epoch_time, char *hms)
{
	assert(hms);
	
    struct tm *utc_time = gmtime(&epoch_time); // For UTC time
    sprintf(hms, "%d:", utc_time->tm_yday);    // num of days
	
    //strftime(hms, sizeof(hms), "%Y-%m-%d %H:%M:%S %Z", utc_time);
    strftime(&hms[strlen(hms)], 80, "%H:%M:%S", utc_time);
	return hms; 
}

char *stpWatchString (stopwatch *who, char *hms)
{
	assert(hms);
	
	time_t epoch_time = stpWatchGetElapsedTime(who);

	hms = secondsToHMS (epoch_time, hms);
	return hms; 
}
//-------------------------------------------------------

#define NTP_TIMEZONE "UTC-8"  // POSIX standard, in which "UTC+0" is UTC London, "UTC-8" is UTC+8 Beijing, "UTC+5" is UTC-5 New York
#define NTP_SERVER1  "0.pool.ntp.org"
#define NTP_SERVER2  "1.pool.ntp.org"
#define NTP_SERVER3  "2.pool.ntp.org"

#include <WiFi.h>

// Different versions of the framework have different SNTP header file names and availability.
#if __has_include(<esp_sntp.h>)
#include <esp_sntp.h>
	#define SNTP_ENABLED 1
#elif __has_include(<sntp.h>)
	#include <sntp.h>
	#define SNTP_ENABLED 1
#endif

#ifndef SNTP_ENABLED
#define SNTP_ENABLED 0
#endif

void initRTCfromNTP(void)
{

	putchar('\n');
	if (!M5.Rtc.isEnabled()) 
	{
		TRACE("FAIL : RTC not found");
		delay(2000);
		assert(M5.Rtc.isEnabled());
	}

	TRACE("cold RTC TIME = %s\n", formattedTimeRTC());

	configTzTime(NTP_TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

	while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) 
	{
		printf(".");
		delay(500);
	}

	TRACE("\nNTP connected\n");

	time_t t = time(nullptr) + 1;  // Advance one second
	while (t > time(nullptr));  // Synchronization in seconds

	M5.Rtc.setDateTime(gmtime(&t));

	TRACE("RTC TIME = %s\n", formattedTimeRTC());
	putchar('\n');
}


