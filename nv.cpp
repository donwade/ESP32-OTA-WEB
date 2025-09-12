
#include "Arduino.h"
#define LED_BUILTIN 2
#include <M5Unified.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#include <ArduinoOTA.h>
#include "RTC.h"
#include "common.h"

#include "nvs_flash.h"
#include "nvs.h"


static SemaphoreHandle_t mMutexNV = xSemaphoreCreateMutex();

static bool bNVSinit = false; 

bool init_NVram(void)
{
	bool retval = false; //fail
	esp_err_t err;

    err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
	{
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
	
    if (err)
    {
		ESP_ERROR_CHECK( err );
		printf("%s FAILED INIT ***** \n", __FUNCTION__);
		assert(err == 0);
    }
	else
	{
		printf("%s up and running\n", __FUNCTION__);
		retval = true;
	}
	return retval;
}

//-------------------------------------------------------------

bool nvErase(void)
{
	esp_err_t err;
	err = nvs_flash_erase();
	TRACE("nvs_flash_erase ret = %d\n", err);
	
	err = nvs_flash_init();
	TRACE("nvs_flash_init retval = %d\n", err);
	
	for (int i = 0; i < 10; i++)
	printf("reseting ....\n");

	esp_restart();
}

bool nvCreateValue(char *name, int32_t value, bool bfast)
{
    nvs_handle hNVhandle;
	esp_err_t err;
	bool retval = false; //fail

	if ( name == NULL ) return false;	
	if ( strlen(name) > 20) name[19] = '\0';

	if (!bfast) xSemaphoreTake(mMutexNV, portMAX_DELAY);
	
    err = nvs_open("storage", NVS_READWRITE, &hNVhandle);
	
    if (err != ESP_OK) 
	{
        TRACE("fail %s %s opening NVS handle!\n", name, esp_err_to_name(err));
    }
	else 
    {
		err = nvs_set_i32(hNVhandle, name, value);
		if (!err)
		{
			err = nvs_commit(hNVhandle);
			if (err)
				TRACE("fail %s %s COMMIT!\n", name, esp_err_to_name(err));
			else
			{
				TRACE("created %s with value 0!\n", name, 0);
				retval = true;
			}
		}
    }

	nvs_close(hNVhandle);
	
	if (!bfast) xSemaphoreGive(mMutexNV);	
	return retval;
}

//-------------------------------------------------------------

bool nvGetValue(char *name, int32_t *value)
{

	int32_t temp;
	
    nvs_handle hNVhandle;
	bool retval = false;
	esp_err_t err;

	if ( name == NULL ) return false;	
	if ( strlen(name) > 20) name[19] = '\0';
	
	xSemaphoreTake(mMutexNV, portMAX_DELAY);

    err = nvs_open("storage", NVS_READWRITE, &hNVhandle);
	
    if (err != ESP_OK) 
	{
        TRACE("fail %s %s open NVS handle!\n", name, esp_err_to_name(err));
		bool test = nvCreateValue(name, *value, true);
		if (test == true)
		{
			retval = true;
		}
		else
			TRACE("failed to create value %s\n", name);
    }
	else 
    {
        err = nvs_get_i32(hNVhandle, name, &temp);
		if (err == ESP_OK)
		{
			*value = temp;
			//TRACE("read %s=%d\n", name, temp);
			retval = true;
		}
		else
		{
			nvCreateValue(name, *value, true); 
		}
    }
	
	nvs_close(hNVhandle);
	xSemaphoreGive(mMutexNV);	
	
	return retval;
}

//-------------------------------------------------------------

bool nvSetValue(char *name, int32_t value)
{
    nvs_handle hNVhandle;
	esp_err_t err;
	bool retval = false; //fail

	if ( name == NULL ) return false;	
	if ( strlen(name) > 20) name[19] = '\0';
	
	xSemaphoreTake(mMutexNV, portMAX_DELAY);
    err = nvs_open("storage", NVS_READWRITE, &hNVhandle);
	
    if (err != ESP_OK) 
	{
        TRACE("fail %s %s opening NVS handle!\n", name, esp_err_to_name(err));
    }
	else 
    {
        err = nvs_set_i32(hNVhandle, name, value);

		if (err)
			TRACE("fail %s %s WRITING!\n", name, esp_err_to_name(err));
		else
		{
			TRACE("pass wrote value %d to %s\n", value, name);
			retval = true; // written.
		}
    }
	
	nvs_close(hNVhandle);
	xSemaphoreGive(mMutexNV);	
	return retval;
}


//-------------------------------------------------------------

bool nvGetSetLtValue(char *name, int32_t *value)
{

	int32_t temp = *value;  // use value is keynot exist
	bool ok;
	
	ok = nvGetValue(name, &temp);
	if (*value < temp)
		ok = nvSetValue(name, *value);
	else
		*value = temp; // get had lesser value
	
	return ok;
}

//-------------------------------------------------------------

bool nvGetSetGtValue(char *name, int32_t *value)
{

	int32_t temp = *value;  // use value is keynot exist
	bool ok;
	
	ok = nvGetValue(name, &temp);
	if ( *value > temp)
		ok = nvSetValue(name, *value);
	else
		*value = temp; // get had greater value
	
	return ok;
}

bool nvIncrementValue(char *name, int32_t *value)
{
	bool retval; 
	int32_t x = *value; // default if doesn't exist;
	
	retval = nvGetValue(name, &x);
	if (!retval) return false;
	
	x++;
	
	retval = nvSetValue(name, x);
	if(value) *value = x;
	return retval;
}


