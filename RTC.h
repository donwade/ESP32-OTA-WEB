extern uint32_t getUTCfromRTC();
extern void  setRTC(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t year);
extern char *format_date_time();

typedef struct
{
     uint32_t started;
     uint32_t requested;
}stopwatch;

extern void stpWatchStart (stopwatch *who);
extern char   *stpWatchString (stopwatch *who, char *hms);
extern time_t stpWatchGetElapsedTime (stopwatch *who);
extern char *secondsToHMS (time_t epoch_time, char *hms);



