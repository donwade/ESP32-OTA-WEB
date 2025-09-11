#define LOCAL_HOSTNAME "m5-core2" 

extern void setupLightSleepByTimer(uint32_t timeMs);
extern void enterLightSleepTimer(void);

//Options are: 240, 160, 120, 80, 40, 20 and 10 MHz
const unsigned int CPU_FREQ = 80;  // can be 240 (default)
const unsigned int CPU_SCALE=240/CPU_FREQ;



