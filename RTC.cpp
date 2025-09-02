#include <M5Unified.h>
#include "RTC.h"
#include "TimeLib.h"

// gps may require cold start assistance from
// the RTC. So we put this routine in the GPS file.


//-----------------------------------------------------------------
char *format_date_time() 
{ //easy way to print date and time
	static char tstring[40]; 
	sprintf(tstring, "%02d/%02d/%4d %02d:%02d:%02d", day(), month(), year(), hour(), minute(), second());
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
		Serial.println(format_date_time());

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

