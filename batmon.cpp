#include <M5Unified.h>
#include "m5Core2-only.h"
#include "viewController.h"
#include "watchdogs.h"
#include "RTC.h"
#include "batmon.h"


static uint32_t _lastTime = 0;
static uint32_t _chargeTime = 0;
static uint32_t _dischargeTime = 0;
static uint32_t _holdchargeTime = 0;

void getBatmon(battmon *who) 
{
	who->chargeTime = _chargeTime; 
	who->dischargeTime = _dischargeTime;
	who->holdchargeTime = _holdchargeTime;
}


void runBatmonTask(void *not_used)
{

	if (!_lastTime) 
	{
		delay(2000);
		Serial.printf("starting %s\n", __FUNCTION__);
		_lastTime = getUTCfromRTC();
	}
	
	uint32_t now =  getUTCfromRTC();
	
	uint32_t diff = now - _lastTime;

	//_chargeTime++;
	//_dischargeTime++;

	batt_stats reply;
	getBatteryStats (&reply);

	if (reply.chargeDirection> 0) 
	{
		colourBarX(_GREEN, 2);
		_chargeTime += diff;
	}
	else if (reply.chargeDirection < 0)
	{
		colourBarX(_RED, 2);
		_dischargeTime += diff;
	}
	else
	{
	  _holdchargeTime += diff;
	   colourBarX(_BLUE, 2);
	}
	_lastTime = now;
	
	Tdelay(1000);		

}

