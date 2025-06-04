#pragma once
#include <defines.h>

#define hkDHT_QUEUE_LEN 5

#if hkDHT11_USE_PIO
    typedef struct DHT_Config_t {
        u8     gpio;
        u8*    data;
        size_t length;
        void*  queue;
        PIO    pio;
        u8     sm;
    } DHT_Config_t;
#else 
    typedef struct DHT_Config_t {
        u8     gpio;
        u8*    data;
        size_t length;
        void*  queue;
    } DHT_Config_t;
#endif

void DHT11_Init(const DHT_Config_t* config);
b8   DHT11_Read(DHT_Config_t* config);

void DHT11_ReadTask(void* pvParameters);