typedef struct 
{
	uint32_t chargeTime;
	uint32_t dischargeTime;
}battmon;

void runBatmonTask(void *not_used);
void getBatmon(battmon *who);


