#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>

#include <defines.h>
#include <comm/onewire/onewire.h>

#define hkDS18B20_JSON_BUFFER   64

enum {    
    DS18B20_CONVERT_T         = 0x44,
    DS18B20_WRITE_SCRATCHPAD  = 0x4E,
    DS18B20_COPY_SCRATCHPAD   = 0x48,
    DS18B20_READ_SCRATCHPAD   = 0xBE,
    DS18B20_RECALL_EE         = 0xB8,
    DS18B20_READ_POWER_SUPPLY = 0xB4
};

enum {
    DS18B20_INIT,
    DS18B20_READ_INIT,
    DS18B20_IDLE,                 
    DS18B20_START_CONVERSION,     
    DS18B20_WAITING_FOR_CONVERSION, 
    DS18B20_START_READ,           
    DS18B20_READING_SCRATCHPAD,   
    DS18B20_PROCESS_DATA,        
    DS18B20_DATA_READY,
    DS18B20_DATA_ERROR,
    DS18B20_DATA_PROCESSED 
};

typedef struct DS18B20_Config_t {
    u64    address;
    u8*    data;
    size_t length;
    f32    temperature;
    u8     dataCount;
    u16    state;
    u8     resolution;
    absolute_time_t convertStartTime;
    QueueHandle_t queue;
} DS18B20_Config_t;

typedef struct DS18B20_DataPacket_t {
    f32  temperature;
    u64  address;
    u32  status;
    json jsonify;
} DS18B20_DataPacket_t;

typedef struct DS18B20_TaskParams_t {
    OneWire_Config_t*     ow;
    DS18B20_Config_t*     ds18b20;
    DS18B20_DataPacket_t* data;
} DS18B20_TaskParams_t;


void vDS18B20_Task(void* pvParameters);

#if hkOW_USE_DMA
    u8 DS18B20_ReadAndProcess(OneWire_Config_t* ow, DS18B20_Config_t* config, DS18B20_DataPacket_t* data);
#else
    u8   DS18B20_ReadAndProcess(OneWire_Config_t* ow, DS18B20_Config_t* config, DS18B20_DataPacket_t* data);
    void DS18B20_Read(DS18B20_Config_t* config);
    void DS18B20_Tick(OneWire_Config_t* ow, DS18B20_Config_t* config, DS18B20_DataPacket_t* data);
    b8   DS18B20_SetResolution(OneWire_Config_t* ow, u8 resolution);
#endif


static char* DS18B20_Jsonify(const void* self) {   
    const DS18B20_DataPacket_t* data = (DS18B20_DataPacket_t*)self;
    
    const char* json = "{"
        "\"sensor\": \"ds18b20\","
        "\"temperature\": %.3f"
    "}";
    
    static char buffer[hkDS18B20_JSON_BUFFER];
    u32 requiredLength = snprintf(buffer, sizeof(buffer), json, data->temperature, data->address);
    if(requiredLength > sizeof(buffer)) {
        HERROR("[DS18B20] Buffer size is to small to send this data packet!\n");
        return NULL;
    }

    return buffer;
}