#pragma once
#include <defines.h>

#define hkDHT_QUEUE_LEN     5
#define hkDHT_JSON_BUFFER   64

typedef enum {
    DHT_INIT,
    DHT_READ_SUCCESS,
    DHT_READ_IN_PROGRESS,
    DHT_READ_BAD_CHECKSUM
} DHT_Status_t;

#if hkDHT_USE_PIO
    #include <hardware/pio.h>
    typedef struct DHT_Config_t {
        u8     gpio;
        u8*    data;
        size_t length;
        void*  queue;
        vu8    status;
        PIO    pio;
        u8     sm;
    } DHT_Config_t;
#else 
    typedef struct DHT_Config_t {
        u8     gpio;
        u8*    data;
        size_t length;
        void*  queue;
        vu8    status;
    } DHT_Config_t;
#endif

typedef char* (*json)(const void* self);
typedef struct DHT_DataPacket_t {
    f32    temperature;
    f32    humidity;
    json   jsonify;
} DHT_DataPacket_t;


void  DHT11_Init(DHT_Config_t* config);
b8    DHT11_Read(DHT_Config_t* config);

void DHT11_ReadTask(void* pvParameters);

static void DHT11_ProcessData(DHT_Config_t* config, DHT_DataPacket_t* data) {
    if(config == NULL || data == NULL) {
        printf("DHT was not properly initialized!\n");
        return;
    }

    u8 checksum = (config->data[0] + config->data[1] +config->data[2] + config->data[3]) & 0xFF; 
    if(checksum != config->data[4]) {
        printf("Data read failed, invalid checksum; Expected: 0x%x ; Got: 0x%x\n", checksum, config->data[4]);
        config->status = DHT_READ_BAD_CHECKSUM;
        return;
    }

    data->humidity    =  config->data[0] + config->data[1] * 0.1f;
    data->temperature =  config->data[2] + config->data[3] * 0.1f;
    if(config->data[2] & 0x80) data->temperature = -data->temperature;
}

static char* DHT11_Jsonify(const void* self) {   
    const DHT_DataPacket_t* data = (DHT_DataPacket_t*)self;
    
    const char* json = "{"
        "\"sensor\": \"dht11\","
        "\"temperature\": %.2f,"
        "\"humidity\": %.2f"
    "}";
    
    static char buffer[hkDHT_JSON_BUFFER];
    u32 requiredLength = snprintf(buffer, sizeof(buffer), json, data->temperature, data->humidity);
    if(requiredLength > sizeof(buffer)) {
        HERROR("[DHT] Buffer size is to small to send this data packet!\n");
        return NULL;
    }

    return buffer;
}