#include "main.h"
#include "cmsis_os.h"
#include "rc522.h"
#include "database.h"
#include <stdio.h>

//外部队列声明，CUBEMX生成
extern osMessageQueueId_t xCardQueueHandle;


uint8_t readCard(uint8_t *readUid, void(*funCallBack)(void))
{
	uint8_t Temp[5];
	if (PCD_Request(0x52, Temp) == 0)
	{
		if (PCD_Anticoll(readUid) == 0)
		{
			if(funCallBack != NULL)
				funCallBack();
			return 0;
		}	
	}
	return 1;
}


void vRC522Task(void *argument) {
    uint8_t readUid[4];
    CardUID_t card_uid;
    
    // ???RC522
    PCD_Reset();
    printf("[RC522] Task Started\r\n");
    
    while(1) {
        // 
        if(!readCard(readUid, NULL)) {
            memcpy(card_uid.data, readUid, 4);
            
            // 
            printf("[RC522] Card detected: %02X-%02X-%02X-%02X\r\n",
                   readUid[0], readUid[1], readUid[2], readUid[3]);
            
            // 
            if(osMessageQueuePut(xCardQueueHandle, &card_uid, 0, 0) == osOK) {
                osDelay(1000);  // 
            } else {
                printf("[RC522] Queue full!\r\n");
            }
        }
        
        osDelay(100);  // 
    }
}