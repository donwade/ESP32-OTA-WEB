#include <M5Unified.h>
#include "m5Core2-only.h"
#include "watchdogs.h"
#include "RTC.h"
#include "batmon.h"


static uint32_t _lastTime = 0;
static uint32_t _chargeTime = 0;
static uint32_t _dischargeTime = 0;

void getBatmon(battmon *who) 
{
	who->chargeTime = _chargeTime/10; 
	who->dischargeTime = _dischargeTime/10;
}


void runBatmonTask(void *not_used)
{

	if (!_lastTime) 
	{
		delay(2000);
		Serial.printf("starting %s\n", __FUNCTION__);
		_lastTime = millis()/100;
	}
	
	uint32_t now = millis()/100;  // tenths of seconds
	
	uint32_t diff = now - _lastTime;

	//_chargeTime++;
	//_dischargeTime++;

	batt_stats reply;
	getBatteryStats (&reply);

	if (reply.chargeDirection> 0) _chargeTime += diff;
	else if (reply.chargeDirection < 0) _dischargeTime += diff;

	_lastTime = now;
	
	Tdelay(1000);		

}

