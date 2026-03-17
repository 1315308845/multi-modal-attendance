#ifndef __AS608_TASK_H
#define __AS608_TASK_H

#include "main.h"
#include <stdint.h>

// ָ��ID�ṹ��
typedef struct {
    uint16_t finger_id;
} FingerID_t;

// ָ��ģ��ʼ������
void AS608_Init(void);

// ָ��ʶ������
uint8_t AS608_Identify(uint16_t *finger_id);

#endif /* __AS608_TASK_H */
