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
extern void enterLightSleep(void);


