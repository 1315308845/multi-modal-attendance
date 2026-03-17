#include "database.h"




static Personnel_t personnel_db[] = {
    {
        .card_uid = {{0x1B, 0x70, 0xD1, 0x06}},
        .finger_id = 0,  
				.face_id = 1, 
        .student_id = "2300110307",
        .name = "hehaoyuan"
    },
		
		{
        .card_uid = {{0x64, 0xC4, 0x64, 0x06}},
        .finger_id = 2, 
				.face_id = 2,
        .student_id = "2300110308", 
        .name = "heyanzhe"
    }
};

static uint8_t db_count = sizeof(personnel_db) / sizeof(Personnel_t);



void DB_Init(void) {
    printf("Database initialized with %d persons\r\n", db_count);
}



static bool uid_equal(const CardUID_t *uid1, const CardUID_t *uid2) {
    return memcmp(uid1->data, uid2->data, UID_SIZE) == 0;
}



const Personnel_t* DB_FindByUID(const CardUID_t *uid) {
    for(uint8_t i = 0; i < db_count; i++) {
        if(uid_equal(uid, &personnel_db[i].card_uid)) {
            return &personnel_db[i];
        }
    }
    return NULL;
}



const Personnel_t* DB_FindByFinger(uint16_t finger_id) {
    for(uint8_t i = 0; i < db_count; i++) {
        if(personnel_db[i].finger_id == finger_id) {
            return &personnel_db[i];
        }
    }
    return NULL;
}

const Personnel_t* DB_FindByFace(uint8_t face_id)
{
    for(int i = 0; i < db_count; i++) {
        if(personnel_db[i].face_id == face_id) {
            return &personnel_db[i];
        }
    }
    return NULL;
}


void DB_PrintAll(void) {
    printf("=== Personnel Database ===\r\n");
    for(uint8_t i = 0; i < db_count; i++) {
        printf("[%d] %s %s - Card: %02X-%02X-%02X-%02X, Finger ID: %d\r\n",
               i, 
               personnel_db[i].student_id,
               personnel_db[i].name,
               personnel_db[i].card_uid.data[0],
               personnel_db[i].card_uid.data[1],
               personnel_db[i].card_uid.data[2],
               personnel_db[i].card_uid.data[3],
               personnel_db[i].finger_id);
    }
    printf("==========================\r\n");
}


