#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>

#include <defines.h>
#include <comm/i2c.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define hkDS1307_JSON_BUFFER   64

enum {
    DS1307_REG_SECONDS   = 0x00,
    DS1307_REG_MINUTES   = 0x01,
    DS1307_REG_HOURS     = 0x02, 
    DS1307_REG_DAY       = 0x03, 
    DS1307_REG_DATE      = 0x04,
    DS1307_REG_MONTH     = 0x05,
    DS1307_REG_YEAR      = 0x06,
    DS1307_REG_CONTROL   = 0x07,
    DS1307_REG_RAM_START = 0x08
};

typedef struct DS1307_Config_t {
    u8 address;
    b8 reset;
    u8 utcOffset;
} DS1307_Config_t;

typedef struct DS1307_DataPacket_t {
    u8  sec;
    u8  min;
    u8  hour;
    u8  dayName; // 1 is Sunday
    u8  day;    
    u8  month;   // It can detect if month have 28, 29, 30 or 31 days 
    u16 year;
    
    u32  timestamp;
    json jsonify;
} DS1307_DataPacket_t;


void vDS1307_Task(void* pvParameters);

i32 DS1307_Init(I2C_Config_t* i2c, DS1307_Config_t* config, DS1307_DataPacket_t* dt);

i32 DS1307_SetTime(I2C_Config_t* i2c, DS1307_Config_t* config, DS1307_DataPacket_t* dt);
i32 DS1307_SetDate(I2C_Config_t* i2c, DS1307_Config_t* config, DS1307_DataPacket_t* dt);
i32 DS1307_SetDateTime(I2C_Config_t* i2c, DS1307_Config_t* config, DS1307_DataPacket_t* dt);

// i32 DS1307_ReadDateTime(I2C_Config_t* i2c, DS1307_Config_t* config, DS1307_DataPacket_t* dt);
u32  DS1307_ReadDateTime(I2C_Config_t* i2c, DS1307_Config_t* config, DS1307_DataPacket_t* dt);
void DS1307_TimeStamp(u32 timestamp, char* buffer, size_t len);


static char* DS1307_Jsonify(const void* self) {   
    const DS1307_DataPacket_t* data = (DS1307_DataPacket_t*)self;
    
    const char* json = "{"
        "\"sensor\": \"ds1307\","
        "\"timestamp\": %u"
    "}";
    
    static char buffer[hkDS1307_JSON_BUFFER];
    u32 requiredLength = snprintf(buffer, sizeof(buffer), json, data->timestamp);
    if(requiredLength > sizeof(buffer)) {
        HERROR("[DS1307] Buffer size is to small to send this data packet!\n");
        return NULL;
    }

    return buffer;
}