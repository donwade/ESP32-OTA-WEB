//#define setCursor(...) M5.Lcd.setCursor(__VA_ARGS__)
//#define print(...) M5.Lcd.print(__VA_ARGS__)
extern void lsetCursor(uint16_t X, uint16_t Y, uint8_t font);
extern void lsetCursor(uint16_t X, uint16_t Y);
extern void lprint(const __FlashStringHelper *x);
extern void lprint(uint8_t x);
extern void lprint(char *x);

extern void lprint(double x, int y);

extern void lprintln(void);
extern void lprintln(char *x);
extern void lclear(void);
extern void setup_M5(void);
extern void lsetTextColor(uint32_t FGND, uint32_t BKGND);
extern uint32_t uptime(void);


#define WIDGET_FONT  &fonts::FreeMonoBoldOblique12pt7b
#define DEFAULT_FONT &fonts::FreeMono12pt7b

//------------------------------------------------------------
typedef struct 
{
	bool isCharging;
	float volt_mV;
	int percent;
	int current_mA;
}batt_stats;

bool getBatteryStats (batt_stats *reply);





