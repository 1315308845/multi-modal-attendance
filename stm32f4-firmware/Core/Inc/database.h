#ifndef __DATABASE_H
#define __DARABASE_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define UID_SIZE 4  


typedef struct {
    uint8_t data[UID_SIZE];
} CardUID_t;



typedef struct {
    CardUID_t card_uid;
    uint16_t finger_id;  
		uint16_t face_id;
    char student_id[11];
    char name[20];
} Personnel_t;




void DB_Init(void);
static bool uid_equal(const CardUID_t *uid1, const CardUID_t *uid2);
const Personnel_t* DB_FindByUID(const CardUID_t *uid);
const Personnel_t* DB_FindByFinger(uint16_t finger_id);  
const Personnel_t* DB_FindByFace(uint8_t face_id);
void DB_PrintAll(void);

#endif

