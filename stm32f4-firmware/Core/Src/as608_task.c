#include "as608_task.h"
#include "as608.h"
#include "database.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>

extern osMessageQueueId_t xFingerQueueHandle;
extern uint32_t AS608Addr;
extern uint8_t aRxBuffer[128];  // ���ջ�����

// ָ��ģ��ʼ������
void AS608_Init(void) {
    uint8_t ensure;
    
    printf("[AS608] Initializing fingerprint module...\r\n");
    
    // ��ջ�����
    extern uint8_t RX_len;
    RX_len = 0;
    memset(aRxBuffer, 0, sizeof(aRxBuffer));
    
    // ��ģ����
    if(GZ_HandShake(&AS608Addr) == 0) {
        printf("[AS608] Handshake OK, Address: 0x%08X\r\n", (unsigned int)AS608Addr);
    } else {
        printf("[AS608] Handshake Failed!\r\n");
        return;
    }
    
    // ��ȡϵͳ����
    SysPara sys_para;
    ensure = GZ_ReadSysPara(&sys_para);
    if(ensure == 0x00) {
        printf("[AS608] Max Fingerprints: %d\r\n", sys_para.GZ_max);
        printf("[AS608] Security Level: %d\r\n", sys_para.GZ_level);
    } else {
        printf("[AS608] Read system parameters failed: 0x%02X\r\n", ensure);
    }
    
    printf("[AS608] Module initialized successfully!\r\n");
}

// ָ��ʶ������
uint8_t AS608_Identify(uint16_t *finger_id) {
    uint8_t ensure;
    SearchResult search_result;
    
    // 1. ¼��ָ��ͼ��
    ensure = GZ_GetImage();
    if(ensure != 0x00) {
        // ֻ�ڷ�0x02(û��ָ��)ʱ������ӡ������Ϣ
        if(ensure != 0x02) {
            printf("[AS608] GetImage failed: 0x%02X - %s\r\n", ensure, EnsureMessage(ensure));
        }
        return ensure;  // û�м�⵽ָ��
    }
    
    printf("[AS608] Image captured successfully\r\n");
    
    // 2. ����������
    ensure = GZ_GenChar(CharBuffer1);
    if(ensure != 0x00) {
        printf("[AS608] GenChar failed: 0x%02X - %s\r\n", ensure, EnsureMessage(ensure));
        return ensure;
    }
    
    printf("[AS608] Character generated successfully\r\n");
    
    // 3. ����ָ�ƿ�
    ensure = GZ_HighSpeedSearch(CharBuffer1, 0, 300, &search_result);
    if(ensure == 0x00) {
        // �ҵ�ƥ���ָ��
        *finger_id = search_result.pageID;
        printf("[AS608] Finger matched! ID: %d, Score: %d\r\n", 
               search_result.pageID, search_result.mathscore);
        return 0x00;
    } else if(ensure == 0x09) {
        // û���ҵ�ƥ���ָ��
        printf("[AS608] No matching fingerprint found\r\n");
        return ensure;
    } else {
        printf("[AS608] Search failed: 0x%02X - %s\r\n", ensure, EnsureMessage(ensure));
        return ensure;
    }
}

// ָ��ʶ������
/*void vFingerTask(void *argument) {
    uint16_t finger_id;
    FingerID_t finger_data;
    uint8_t ensure;
    
    // ��ʼ��AS608ģ��
    osDelay(1000);  // �ȴ�ϵͳ�ȶ�
    AS608_Init();
    
    printf("[Finger] Task Started\r\n");
    
    while(1) {
        // ��ѯʽʶ��ָ��
        ensure = AS608_Identify(&finger_id);
        
        if(ensure == 0x00) {
            // ʶ���ɹ�
            finger_data.finger_id = finger_id;
            
            // ���͵�ָ�Ӷ���
            if(osMessageQueuePut(xFingerQueueHandle, &finger_data, 0, 0) == osOK) {
                printf("[Finger] Finger ID %d sent to queue\r\n", finger_id);
                osDelay(2000);  // ��ֹ�ظ�ʶ��
            } else {
                printf("[Finger] Queue full!\r\n");
            }
        }
        
        osDelay(500);  // ��ѯ���500ms
    }
}*/
