#pragma once
#include <defines.h>

#define hkDHT_QUEUE_LEN 5

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

void DHT11_Init(DHT_Config_t* config);
b8   DHT11_Read(DHT_Config_t* config);

void DHT11_ReadTask(void* pvParameters);