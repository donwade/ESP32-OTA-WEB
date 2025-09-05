extern void trace(const char *fn, uint32_t line, ...);

#define TRACE(...) trace(__FUNCTION__, __LINE__, ##__VA_ARGS__)


