#ifndef VIEW_CONTROLLER
#define VIEW_CONTROLLER

//#include "cppQueue.h"
#include <ArduinoQueue.h>

#define _BLACK       0x000000
#define _WHITE       0xFFFFFF
#define _RED         0xFF0000
#define _GREEN       0x00FF00
#define _BLUE        0x0000FF

#define _CYAN        (_GREEN | _BLUE)
#define _MAGENTA     (_RED   | _BLUE)
#define _YELLOW      (_RED   | _GREEN)

#define _ORANGE      (_RED | (165 << 8))
#define _NAVY        (_BLUE/2)
#define _MAROON      (_RED/2)
#define _DGREEN      (_GREEN/2)
#define _DCYAN       (_CYAN/2)
#define _OLIVE       ( (_RED/2) | (_GREEN/2))
#define _PINK        ( _RED | (_GREEN/20) | (_BLUE/20))

extern void colourBar(uint8_t R,uint8_t G, uint8_t B);
extern void colourBarX(uint32_t RGB, uint8_t pct);
extern void colourNleds(uint8_t who, uint8_t width, uint8_t R,uint8_t G, uint8_t B);
extern void setToggleColors(uint32_t RGB_LEFT, uint32_t RGB_RIGHT, uint16_t pct =100) ;


typedef enum KEY_STATE { KEY_UNKNOWN, KEY_DOWN, KEY_UP};

extern int  xprintf(uint8_t lineNo, const char *format, ...); 
extern int  cprintf(uint32_t color, uint8_t lineNo, const char *format, ...);
extern void lfillRect(uint16_t x, uint16_t y, uint16_t wide, uint16_t height, uint32_t RGB);

extern void runLightBarTask(void *not_used);

void touchPanel_impl(void);
void setup_button(void);
void button_create(void);

extern void showPower(void);

typedef enum { 
	DISPLAY_REFRESH, //nothing
	LBUTTON_DN, LBUTTON_UP, // odd DOWN. Even UP
	MBUTTON_DN, MBUTTON_UP, 
	RBUTTON_DN, RBUTTON_UP,
    BUTTON_INIT    } BUTTON_EVENT;


void threeButtonMenu(
	char *leftButtonText, 
	char *middleButtonText, 
	char *rightButtonText 
	);


void twoButtonMenu(
    char *leftButtonText, 
    char *rightButtonText 
    );

void threeButtonText(
	char *leftButtonText, 
	char *middleButtonText, 
	char *rightButtonText);

extern void button_push(BUTTON_EVENT msg);
extern BUTTON_EVENT button_pop(uint32_t maxWaitMs);


#endif
