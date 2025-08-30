#include <M5Unified.h>
#include "m5Core2-only.h"
#include "viewController.h"
#include "RTC.h"

static uint8_t vert = 0;

void lsetCursor(uint16_t X, uint16_t Y)
{
	//int16_t horz =M5.Lcd.textWidth(" ", font);
	//printf("charX=%d\n", horz);

	if (!vert) vert = M5.Lcd.fontHeight() + 3;
	//printf("charY=%d\n", vert);
	
	M5.Lcd.setCursor(X, Y * vert);
}

void lprint(const __FlashStringHelper *x)
{
	M5.Lcd.print(x);
}

void lprint(char *x)
{
	M5.Lcd.print(x);
}

void lprint(uint8_t x)
{
	M5.Lcd.print(x);
}


void lprint(double x, int y)
{
	M5.Lcd.print(x,y);
}

void lprintln(void)
{
	M5.Lcd.println();
}

void lprintln(char *x)
{
	M5.Lcd.println(x);
}

void lclear(void)
{
	M5.Lcd.clear();
}
//--------------------------------------------------
#include <Adafruit_GFX.h>    // Core graphics library
// below must follow above
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMono12pt7b.h>

//--------------------------------------------------
bool getBatteryStats (batt_stats *reply)
{
	const float fMAX_BAT_mV = 4200; 
	const float fMIN_BAT_mV = 3100;
	const float fRange = fMAX_BAT_mV - fMIN_BAT_mV;
	
	bool isCharging = M5.Power.isCharging();

	float volt_mV = M5.Power.getBatteryVoltage();
	int percent = (volt_mV - fMIN_BAT_mV) * 100. / fRange;

	int current_mA = M5.Power.getBatteryCurrent();

	reply->isCharging = isCharging;
	reply->current_mA = current_mA;
	reply->percent = percent;
	reply->volt_mV = volt_mV;
	
	return true;
}
//--------------------------------------------------
static uint32_t startUTC;

void setup_M5(void)
{
	M5.begin();
	M5.Power.setExtOutput(true);  // enable external bus

	// confusing. this sets font for background display
    M5.Lcd.setTextFont(DEFAULT_FONT);

	// confusing. this sets font for buttons
	M5.Lcd.setFont(WIDGET_FONT);
	
	//M5.Speaker.setAllChannelVolume(100);
	
	lfillRect(0, 0, 50, 50, 0x0000FF);
	delay(2000);

	Serial.begin(115200);
		
	setup_button();
	
	startUTC = getUTCfromRTC();
}
//-------------------------------------------------------------
uint32_t uptime(void)
{
	return getUTCfromRTC() - startUTC;
}

void lsetTextColor(uint32_t FGND, uint32_t BKGND)
{
    M5.Lcd.setTextFont(DEFAULT_FONT);
	//M5.Lcd.setTextColor(FGND);
	M5.Lcd.setTextColor(FGND,BKGND);
	
	//w = M5.Lcd.width();
    //h = M5.Lcd.height();
    //M5.Lcd.setTextColor(TFT_BLACK);
    //M5.Lcd.fillScreen(WHITE);
    //M5.Display.setRotation(1);
    //M5.Display.setTextColor(TFT_BLACK);
    //M5.Display.setTextDatum(top_center);
    //M5.Display.drawString("HI MOM", w / 2, 0, &fonts::FreeMonoBold12pt7b);
    
}


void reportPower(void)
{
    //M5.Display.clear(TFT_WHITE);
    
    bool isCharging = M5.Power.isCharging();
    int vol_per = M5.Power.getBatteryLevel();
    int vol = M5.Power.getBatteryVoltage();
    int cur = M5.Power.getBatteryCurrent();

	Serial.printf("\n-------------------------\n");
    Serial.printf("Charging: %s\n", isCharging ? "Yes" : "No");
    Serial.printf("Bat_level: %d%%\n", vol_per);
    Serial.printf("Bat_voltage: %d%mV\n", vol);
    Serial.printf("Bat_current: %d%mA\n\n", cur);
}                                              

