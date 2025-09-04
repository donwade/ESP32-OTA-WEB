#define LOCAL_HOSTNAME "m5-core2" 

extern bool nvCreateValue(char *name, int32_t value);
extern bool nvGetValue(char *name, int32_t *value);
extern bool nvSetValue(char *name, int32_t value);
extern bool nvIncrementValue(char *name, int32_t *value);
extern bool nvErase(void);



