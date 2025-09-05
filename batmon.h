typedef struct 
{
	uint32_t chargeTime;
	uint32_t dischargeTime;
    uint32_t holdchargeTime;

}battmon;

void runBatmonTask(void *not_used);
void getBatmon(battmon *who); 


