#include <M5Unified.h>
#include <FastLED.h>                                                                                                                                                                    
#include <m5Core2-only.h>
#include "viewController.h"
#include "watchdogs.h"
#include "RTC.h"


#define LEDS_PIN 25
#define LEDS_NUM 10

static bool bStopLedBarToggle = true;
CRGB ledsBuff[LEDS_NUM];

static SemaphoreHandle_t displayMutex = xSemaphoreCreateMutex();
static SemaphoreHandle_t keyCountingSemaphore;

static bool bStopToggle = true;
//------------------------------------------------------
// make entire LED bar one colour
void colourBar(uint8_t R,uint8_t G, uint8_t B) 
{
	bStopToggle = true;
	
	for (int i = 0; i < LEDS_NUM; i++) {
		ledsBuff[i].setRGB(G, R, B);
	}
	FastLED.show();
}	

//------------------------------------------------------
// make entire LED bar one colour
void colourBarX(uint32_t RGB, uint8_t pct) 
{
	uint16_t R,G,B;
	G = ((RGB >> 16) & 0xFF) * pct /100;
	R = ((RGB >>  8) & 0xFF) * pct /100;
	B =  (RGB 	     & 0xFF) * pct /100;

	bStopToggle = true;
	
	for (int i = 0; i < LEDS_NUM; i++) {
		ledsBuff[i].setRGB(R, G, B);
	}
	FastLED.show();
}	
//------------------------------------------------------
// make range of LEDs one color
void colourNleds(uint8_t who, uint8_t width, uint8_t R,uint8_t G, uint8_t B) 
{
	assert(who < LEDS_NUM);
	assert(who + width < LEDS_NUM);
	bStopToggle = true;
	
	for (int i = who; i < who + width; i++) {
		ledsBuff[i].setRGB(R, G, B);
	}
	FastLED.show();
}	
//------------------------------------------------------
// make entire LED bar one colour

static void toggleLeftRight(uint32_t RGB_LEFT, uint32_t RGB_RIGHT) 
{
	uint32_t R,G,B;

	R = ((RGB_LEFT >> 16) & 0xFF);
	G = ((RGB_LEFT >>  8) & 0xFF);
	B =  (RGB_LEFT 	     & 0xFF);
	
	for (int i = 0; i < LEDS_NUM/2; i++) {
		ledsBuff[i].setRGB(R, G, B);
	}

	R = ((RGB_RIGHT >> 16) & 0xFF);
	G = ((RGB_RIGHT >>  8) & 0xFF);
	B =  (RGB_RIGHT 	   & 0xFF);
	
	for (int i = LEDS_NUM/2 ; i < LEDS_NUM; i++) {
		ledsBuff[i].setRGB(R, G, B);
	}

	FastLED.show();
}	


//---------------------------------------------------------
static uint32_t colourBarLeft = _GREEN;
static uint32_t colourBarRight = _PINK;


void setToggleColors(uint32_t RGB_LEFT, uint32_t RGB_RIGHT, uint16_t brite)
{
	uint32_t r,g,b;
	
	colourBarLeft = RGB_LEFT;
	g = (RGB_LEFT >> 16 ) & 0xFF;
	r = (RGB_LEFT >> 8  ) & 0xFF;
	b = (RGB_LEFT       ) & 0xFF;

	r = r * brite/100;
	g = g * brite/100;
	b = b * brite/100;
	RGB_LEFT = r << 16 | g << 8 | b;

	g = (RGB_RIGHT >> 16 ) & 0xFF;
	r = (RGB_RIGHT >> 8  ) & 0xFF;
	b = (RGB_RIGHT       ) & 0xFF;

	r = r * brite/100;
	g = g * brite/100;
	b = b * brite/100;
	RGB_RIGHT = r << 16 | g << 8 | b;
	
	colourBarLeft = RGB_LEFT;
	colourBarRight = RGB_RIGHT;

	//Serial.printf("left=0x%08X right=0x%08X\n", RGB_LEFT, RGB_RIGHT);
	
	bStopLedBarToggle = (RGB_LEFT == _BLACK && RGB_RIGHT == _BLACK) ? true : false;


	bStopLedBarToggle = false;
}


#undef ENABLE_DEBUG_PING
#undef  LOG_LEVEL_INFO		  // mute ping results  
#define LOG_LEVEL_WARN 

#include <ESPping.h>

char pingStatus[30] ="pingStatus";

void runPingTask(void *not_used)
{
	const IPAddress targetIP(10, 0, 0, 1);  

	kickDog();
	bool success = Ping.ping(targetIP, 2); // Or Ping.ping(targetHost, 5);

	if (success) 
	{
		//Serial.print("Average response time: ");
		//Serial.print(Ping.averageTime());
		//Serial.println(" ms");
		sprintf(pingStatus, "Ping %5.3f mS ",  Ping.averageTime()); 
	}
	else
	{
		sprintf(pingStatus, "Ping fail");
		Serial.println("Ping failed.");  
	}		
	delay(2000);
}

void runLightBarTask(void *not_used)
{
	static bool bFlipper;
	
	if (!bStopLedBarToggle)
	{
		if (bFlipper)
			toggleLeftRight(colourBarLeft,  colourBarRight);
		else
			toggleLeftRight(colourBarRight, colourBarLeft);
		bFlipper = !bFlipper;
	}
	Tdelay(500);
}
//-------------------------------------------------------------
void lfillRect(uint16_t x, uint16_t y, uint16_t wide, uint16_t height, uint32_t RGB)
{
	xSemaphoreTake(displayMutex, portMAX_DELAY);
	
	uint16_t w = wide ? wide : M5.Lcd.width();
    uint16_t h = height ? height : M5.Lcd.height();
    M5.Lcd.writeFillRectPreclipped(x, y, w, h, 0x00FFFF);
	
    //M5.Lcd.fillScreen(WHITE);
	
	xSemaphoreGive(displayMutex);	
}


//---------------------------------------------------------
int  xprintf(uint8_t lineNo, const char *format, ...) 
{
	static u_int8_t linelen[10];
	va_list args;
	va_start(args, format);
	char buffer[40];
	
	vsnprintf(buffer, sizeof(buffer)-1, format, args);

	xSemaphoreTake(displayMutex, portMAX_DELAY);

	lsetTextColor(_WHITE, _BLACK);
	lsetCursor(0, lineNo); 

	// time to kill off any chars from old print
	uint32_t ll = strlen(buffer);
	
	if ( ll < linelen[lineNo])
	{
		// overstrike past text with spaces if needed
		uint32_t add = linelen[lineNo] - ll;

		// mono spaced font right :) I'm lazy.
		for (int i = 0; i < add+1; i++) strcat(buffer," ");
	}
	
	linelen[lineNo] = ll;
	lprint(buffer);

	xSemaphoreGive(displayMutex);	
	va_end(args);
	return 0;
}

int  cprintf(uint32_t color, uint8_t lineNo, const char *format, ...) 
{
	static u_int8_t linelen[10];
	va_list args;
	va_start(args, format);
	char buffer[40];
	
	vsnprintf(buffer, sizeof(buffer)-1, format, args);

	xSemaphoreTake(displayMutex, portMAX_DELAY);
	
	lsetTextColor(color, _BLACK);
	lsetCursor(0, lineNo); 

	// time to kill off any chars from old print
	uint32_t ll = strlen(buffer);
	
	if ( ll < linelen[lineNo])
	{
		// overstrike past text with spaces if needed
		uint32_t add = linelen[lineNo] - ll +1 ; //+1 doesnt clear 100.00%

		// mono spaced font right :) I'm lazy.
		for (int i = 0; i < add+1; i++) strcat(buffer," ");
	}
	
	linelen[lineNo] = ll;
	lprint(buffer);

	lsetTextColor(_WHITE, _BLACK);

	xSemaphoreGive(displayMutex);	
	va_end(args);
	return 0;
}





m5::touch_detail_t touchDetail;
LGFX_Button buttonLeft, buttonMiddle, buttonRight;

static unsigned char buttonWidth = 60;
static unsigned char buttonHeight = 60;

static unsigned int phyDispWidth;
static unsigned int phyDispHeigth;

int8_t leftButtonState = -1;
int8_t rightButtonState = -1;
int8_t middleButtonState = -1;

static uint32_t bMenuIsActive = false;


//--------------------------------------------------
void showPower(void)
{
	static uint8_t flipper;
	
	if (flipper++ & 0x10)
	{
		bool isCharging = M5.Power.isCharging();
		int percent = M5.Power.getBatteryVoltage() * 100/ 3700; // 3.7 v bat max
		int vol = M5.Power.getBatteryVoltage();
		int cur = M5.Power.getBatteryCurrent();
		
		cprintf(_YELLOW, 6, "%3.1fv %03d%% %4dmA %s", 
							(float)vol/1000.,
							percent,
							cur,
							isCharging ? "CHG":"DIS");
	}
	else
	{
		cprintf(_YELLOW, 6, "%s", formattedTimeRTC());
	}
}	

//--------------------------------------------------

void threeButtonText(
	char *leftButtonText, 
	char *middleButtonText, 
	char *rightButtonText)
{
	xSemaphoreTake(displayMutex, portMAX_DELAY);

	if (leftButtonText) buttonLeft.setLabelText(leftButtonText);
	if (rightButtonText) buttonRight.setLabelText(rightButtonText);
	if (middleButtonText) buttonMiddle.setLabelText(middleButtonText);

	buttonLeft.drawButton();
	buttonRight.drawButton();
	buttonMiddle.drawButton();
	
	xSemaphoreGive(displayMutex);	
	
};


void threeButtonMenu(
	char *leftButtonText, 
	char *middleButtonText, 
	char *rightButtonText)
{
	xSemaphoreTake(displayMutex, portMAX_DELAY);

    M5.Lcd.setTextFont(WIDGET_FONT);

	bMenuIsActive = true;
	buttonWidth = phyDispWidth /3;
	
	// coordinates specify center of button hence odd math
	buttonLeft.initButton(&M5.Lcd,	buttonWidth * 0 + buttonWidth/2, 210, buttonWidth, buttonHeight, TFT_WHITE, TFT_GREEN, TFT_BLACK, leftButtonText, 1, 1);
	buttonLeft.drawButton();
	leftButtonState = KEY_UNKNOWN;
	
	buttonMiddle.initButton(&M5.Lcd, buttonWidth * 1 + buttonWidth/2, 210, buttonWidth, buttonHeight, TFT_WHITE, TFT_YELLOW, TFT_BLACK, middleButtonText, 1, 1);
	buttonMiddle.drawButton();
    middleButtonState = KEY_UNKNOWN;

	buttonRight.initButton(&M5.Lcd, buttonWidth * 2 + buttonWidth/2 ,210, buttonWidth, buttonHeight, TFT_WHITE, TFT_RED, TFT_BLACK, rightButtonText, 1, 1);
	buttonRight.drawButton();
	rightButtonState = KEY_UNKNOWN;

	BUTTON_EVENT msg;

	xSemaphoreGive(displayMutex);	
	
}

#define MAX_KEYS_QUEUED 8

static ArduinoQueue<BUTTON_EVENT> buttonQueue(20);

void button_push(BUTTON_EVENT msg)
{
	buttonQueue.enqueue(msg);
	xSemaphoreGive(keyCountingSemaphore);
}

BUTTON_EVENT button_pop(uint32_t maxWaitMs)
{
	BUTTON_EVENT abutton;

	kickDog();
	while (xSemaphoreTake( keyCountingSemaphore, pdMS_TO_TICKS(maxWaitMs) ) == pdTRUE)
	{
		abutton = buttonQueue.dequeue();
		return abutton;
	}
		
	return DISPLAY_REFRESH;
}

void setup_button()
{

	FastLED.addLeds<SK6812, LEDS_PIN>(ledsBuff, LEDS_NUM);	

	keyCountingSemaphore = xSemaphoreCreateCounting(MAX_KEYS_QUEUED,0);

	phyDispWidth = M5.Lcd.width();
	phyDispHeigth = M5.Lcd.height();

	threeButtonMenu("LEFT", "MIDDLE", "RIGHT");
	
	BUTTON_EVENT msg;
	
	msg = BUTTON_INIT;
}

void touchPanel_impl()
{
	BUTTON_EVENT msg;
	kickDog();
	delay(200); // poor man debounce
	
	if (!bMenuIsActive)
	{
		delay(1);
		return;
	}
	
	xSemaphoreTake(displayMutex, portMAX_DELAY);
	
	// don't update if menu not running.
	M5.update();

	touchDetail = M5.Touch.getDetail();

	if (touchDetail.isPressed())
	{
		
		if(buttonLeft.contains(touchDetail.x, touchDetail.y))
		{
			if (leftButtonState != KEY_DOWN)
			{
				leftButtonState = KEY_DOWN;

				M5.Speaker.setVolume(64);
				M5.Speaker.tone(2000, 50);				
				M5.Speaker.setVolume(128);

				msg = LBUTTON_DN;
				button_push(msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		else if(buttonMiddle.contains(touchDetail.x, touchDetail.y))
		{
			if (middleButtonState != KEY_DOWN)
			{
				middleButtonState = KEY_DOWN;
				
				M5.Speaker.setVolume(64);
				M5.Speaker.tone(2000, 50);				
				M5.Speaker.setVolume(128);

				msg = MBUTTON_DN;
				button_push(msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		else if(buttonRight.contains(touchDetail.x, touchDetail.y))
		{
			if (rightButtonState != KEY_DOWN)
			{
				rightButtonState = KEY_DOWN;

				M5.Speaker.setVolume(64);
				M5.Speaker.tone(2000, 50);				
				M5.Speaker.setVolume(128);
				
				msg = RBUTTON_DN;
				button_push(msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		
	}

	if (touchDetail.isReleased())
	{
			
		if(buttonLeft.contains(touchDetail.x, touchDetail.y))
		{
			if ( leftButtonState != KEY_UP)
			{
				leftButtonState = KEY_UP;

				msg = LBUTTON_UP;
				button_push(msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		else if(buttonMiddle.contains(touchDetail.x, touchDetail.y))
		{
			if (middleButtonState != KEY_UP)
			{
				middleButtonState = KEY_UP;

				msg = MBUTTON_UP;
				buttonQueue.enqueue(msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
		else if(buttonRight.contains(touchDetail.x, touchDetail.y))
		{
			if (rightButtonState != KEY_UP)
			{
				rightButtonState = KEY_UP;

				msg = RBUTTON_UP;
				buttonQueue.enqueue(msg);
				xSemaphoreGive(keyCountingSemaphore);
			}
		}
	}

	xSemaphoreGive(displayMutex);	

}

