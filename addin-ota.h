#define LOCAL_HOSTNAME "m5-core2" 

extern bool nvCreateValue(char *name, int32_t value);
extern bool nvGetValue(char *name, int32_t *value);
extern bool nvSetValue(char *name, int32_t value);
extern bool nvIncrementValue(char *name, int32_t *value);
extern bool nvErase(void);

extern bool nvCreateValue(char *name, int32_t value, bool bfast = false);
extern bool nvGetSetGtValue(char *name, int32_t *value);
extern bool nvGetSetLtValue(char *name, int32_t *value);

extern void setupSleepByGPIO(gpio_num_t wakeupPin);
extern void enterLightSleepGPIO(void);

extern void setupLightSleepByTimer(uint32_t timeMs);
extern void enterLightSleepTimer(void);

//Options are: 240, 160, 120, 80, 40, 20 and 10 MHz
const unsigned int CPU_FREQ = 80;  // can be 240 (default)



