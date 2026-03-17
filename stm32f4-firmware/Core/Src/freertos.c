/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "rc522.h"
#include "database.h"
#include "as608_task.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// readCard������rc522_task.c�ж���
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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern uint8_t aRxBuffer[128];  
extern uint32_t AS608Addr;
extern uint8_t RX_len;          
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for RC522_Task */
osThreadId_t RC522_TaskHandle;
const osThreadAttr_t RC522_Task_attributes = {
  .name = "RC522_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for attendance_Task */
osThreadId_t attendance_TaskHandle;
const osThreadAttr_t attendance_Task_attributes = {
  .name = "attendance_Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Print_Task */
osThreadId_t Print_TaskHandle;
const osThreadAttr_t Print_Task_attributes = {
  .name = "Print_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Finger_Task */
osThreadId_t Finger_TaskHandle;
const osThreadAttr_t Finger_Task_attributes = {
  .name = "Finger_Task",
  .stack_size = 384 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for FaceRxTask */
osThreadId_t FaceRxTaskHandle;
const osThreadAttr_t FaceRxTask_attributes = {
  .name = "FaceRxTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for xCardQueue */
osMessageQueueId_t xCardQueueHandle;
const osMessageQueueAttr_t xCardQueue_attributes = {
  .name = "xCardQueue"
};
/* Definitions for PrintQueue */
osMessageQueueId_t PrintQueueHandle;
const osMessageQueueAttr_t PrintQueue_attributes = {
  .name = "PrintQueue"
};
/* Definitions for xFingerQueue */
osMessageQueueId_t xFingerQueueHandle;
const osMessageQueueAttr_t xFingerQueue_attributes = {
  .name = "xFingerQueue"
};
/* Definitions for xFaceQueue */
osMessageQueueId_t xFaceQueueHandle;
const osMessageQueueAttr_t xFaceQueue_attributes = {
  .name = "xFaceQueue"
};
/* Definitions for xDbMutex */
osMutexId_t xDbMutexHandle;
const osMutexAttr_t xDbMutex_attributes = {
  .name = "xDbMutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void vRC522Task(void *argument);
void vAttendanceTask(void *argument);
void vPrintTask(void *argument);
void vFingerTask(void *argument);
void vFaceRxTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of xDbMutex */
  xDbMutexHandle = osMutexNew(&xDbMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of xCardQueue */
  xCardQueueHandle = osMessageQueueNew (10, 4, &xCardQueue_attributes);

  /* creation of PrintQueue */
  PrintQueueHandle = osMessageQueueNew (20, 80, &PrintQueue_attributes);

  /* creation of xFingerQueue */
  xFingerQueueHandle = osMessageQueueNew (10, 2, &xFingerQueue_attributes);

  /* creation of xFaceQueue */
  xFaceQueueHandle = osMessageQueueNew (4, sizeof(uint8_t), &xFaceQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of RC522_Task */
  RC522_TaskHandle = osThreadNew(vRC522Task, NULL, &RC522_Task_attributes);

  /* creation of attendance_Task */
  attendance_TaskHandle = osThreadNew(vAttendanceTask, NULL, &attendance_Task_attributes);

  /* creation of Print_Task */
  Print_TaskHandle = osThreadNew(vPrintTask, NULL, &Print_Task_attributes);

  /* creation of Finger_Task */
  Finger_TaskHandle = osThreadNew(vFingerTask, NULL, &Finger_Task_attributes);

  /* creation of FaceRxTask */
  FaceRxTaskHandle = osThreadNew(vFaceRxTask, NULL, &FaceRxTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_vRC522Task */
/**
  * @brief  Function implementing the RC522_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_vRC522Task */
void vRC522Task(void *argument)
{
  /* USER CODE BEGIN vRC522Task */
	
	 uint8_t readUid[4];
    CardUID_t card_uid;
    
    
    PCD_Reset();
    printf("[RC522] Task Started\r\n");
	
  /* Infinite loop */
  for(;;)
  {  
		 if(!readCard(readUid, NULL)) {
         
            memcpy(card_uid.data, readUid, 4);
            
          
            printf("[RC522] Card detected: %02X-%02X-%02X-%02X\r\n",
                   readUid[0], readUid[1], readUid[2], readUid[3]);
            
          
            if(osMessageQueuePut(xCardQueueHandle, &card_uid, 0, 0) == osOK) {
                osDelay(1000);  
            } else {
                printf("[RC522] Queue full!\r\n");
            }
        }
        
    osDelay(500);  // ��ѯ���100ms
    osDelay(1);
  }
  /* USER CODE END vRC522Task */
}

/* USER CODE BEGIN Header_vAttendanceTask */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vAttendanceTask */
void vAttendanceTask(void *argument)
{
  /* USER CODE BEGIN vAttendanceTask */
	 CardUID_t card_uid;
    FingerID_t finger_data;
    const Personnel_t *person;
    char msg[80];
	  printf("[Attendance] Task Started\r\n");
	  uint8_t face_id;
  /* Infinite loop */
  for(;;)
  { 
      
        
        
        if(osMessageQueueGet(xCardQueueHandle, &card_uid, NULL, 0) == osOK) {
        
            person = DB_FindByUID(&card_uid);
            
            if(person != NULL) {
           
                snprintf(msg, sizeof(msg), 
                        "[Card] Welcome, %s %s! (Card: %02X-%02X-%02X-%02X)\r\n",
                        person->student_id, 
                        person->name,
                        card_uid.data[0], card_uid.data[1],
                        card_uid.data[2], card_uid.data[3]);
            } else {
             
                snprintf(msg, sizeof(msg),
                        "[Card] Visitor, please contact administrator! "
                        "(Card: %02X-%02X-%02X-%02X)\r\n",
                        card_uid.data[0], card_uid.data[1],
                        card_uid.data[2], card_uid.data[3]);
            }
            
           
            osMessageQueuePut(PrintQueueHandle, msg, 0, 0);
        }
        
        
        if(osMessageQueueGet(xFingerQueueHandle, &finger_data, NULL, 0) == osOK) {
           
            person = DB_FindByFinger(finger_data.finger_id);
            
            if(person != NULL) {
                
                snprintf(msg, sizeof(msg), 
                        "[Finger] Welcome, %s %s! (Finger ID: %d)\r\n",
                        person->student_id, 
                        person->name,
                        finger_data.finger_id);
            } else {
               
                snprintf(msg, sizeof(msg),
                        "[Finger] Unregistered fingerprint! (Finger ID: %d)\r\n",
                        finger_data.finger_id);
            }
            
            
            osMessageQueuePut(PrintQueueHandle, msg, 0, 0);
        }
		// 检查人脸队列（非阻塞，0超时）
if(osMessageQueueGet(xFaceQueueHandle, &face_id, NULL, 0) == osOK) {
    
    const Personnel_t* person = DB_FindByFace(face_id);
    char msg[80];
    
    if(person != NULL) {
        snprintf(msg, sizeof(msg), 
                "[Face] Welcome, %s %s! (Face ID: %d)\r\n",
                person->student_id, 
                person->name,
                face_id);
    } else {
        snprintf(msg, sizeof(msg),
                "[Face] Please contact administrator! (Face ID: %d)\r\n",
                face_id);
    }
    
    osMessageQueuePut(PrintQueueHandle, msg, 0, 0);
}
        osDelay(10);  
  }
  /* USER CODE END vAttendanceTask */
}

/* USER CODE BEGIN Header_vPrintTask */
/**
* @brief Function implementing the Print_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vPrintTask */




void vPrintTask(void *argument)
{
  /* USER CODE BEGIN vPrintTask */
	char msg[80];
    
    // 启动提示也发给ESP32（如果ESP32已就绪，会在串口助手显示这行）
    HAL_UART_Transmit(&huart2, (uint8_t*)"[STM32] PrintTask Started\r\n", 27, 100);
    
  /* Infinite loop */
  for(;;)
  {	
		if(osMessageQueueGet(PrintQueueHandle, msg, NULL, osWaitForever) == osOK) {
            // 统一走USART2给ESP32
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 200);
        }
    osDelay(1);
  }
  /* USER CODE END vPrintTask */
}







//void vPrintTask(void *argument)
//{
  /* USER CODE BEGIN vPrintTask */
//	char msg[80];
    
//    printf("[Print] Task Started\r\n");
  /* Infinite loop */
//  for(;;)
//  {	
//		if(osMessageQueueGet(PrintQueueHandle, msg, NULL, osWaitForever) == osOK) {
            // 通过USART2输出
 //           printf("%s", msg);
//        }
//    osDelay(1);
//  }
  /* USER CODE END vPrintTask */
//}

/* USER CODE BEGIN Header_vFingerTask */
/**
* @brief Function implementing the Finger_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vFingerTask */
void vFingerTask(void *argument)
{
  /* USER CODE BEGIN vFingerTask */
    uint16_t finger_id;
    FingerID_t finger_data;
    uint8_t ensure;
    
  
    osDelay(1000);  // �ȴ�ϵͳ�ȶ�
    AS608_Init();
    
    printf("[Finger] Task Started\r\n");
    
  /* Infinite loop */
  for(;;)
  {
       
        ensure = AS608_Identify(&finger_id);
        
        if(ensure == 0x00) {
            // ʶ���ɹ�
            finger_data.finger_id = finger_id;
            
            
            if(osMessageQueuePut(xFingerQueueHandle, &finger_data, 0, 0) == osOK) {
                printf("[Finger] Finger ID %d sent to queue\r\n", finger_id);
                osDelay(2000); 
            } else {
                printf("[Finger] Queue full!\r\n");
            }
        }
        
        osDelay(500);  // ��ѯ���500ms
  }
  /* USER CODE END vFingerTask */
}

/* USER CODE BEGIN Header_vFaceRxTask */
/**
* @brief Function implementing the FaceRxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vFaceRxTask */
void vFaceRxTask(void *argument)
{
  /* USER CODE BEGIN vFaceRxTask */
  uint8_t face_id;
  printf("[Face] RX Task Started\r\n");
  /* Infinite loop */
  for(;;)
  {
		 // 直接收一个字节
        if(HAL_UART_Receive(&huart3, &face_id, 1, 100) == HAL_OK) {
            printf("[Face] RX ID: %d\r\n", face_id);
            
            // 发送到队列
            if(osMessageQueuePut(xFaceQueueHandle, &face_id, 0, 0) != osOK) {
                printf("[Face] Queue full!\r\n");
            }
        }
		
    osDelay(10);
  }
  /* USER CODE END vFaceRxTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

