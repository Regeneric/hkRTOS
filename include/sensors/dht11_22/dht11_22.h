#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <defines.h>
#include <stdio.h>
#include <math.h>
#include <core/logger.h>


#define hkDHT_JSON_BUFFER   96

enum {
    DHT_INIT,
    DHT_READ_INIT,
    DHT_READ_SUCCESS,
    DHT_READ_IN_PROGRESS,
    DHT_READ_BAD_CHECKSUM,
    DHT_11,
    DHT_22
};

#if hkDHT_USE_PIO
    #include <hardware/pio.h>
    typedef struct DHT_Config_t {
        u8     gpio;
        u8*    rawData;
        size_t length;
        vu8    status;
        u8     type;
        PIO    pio;
        u8     sm;
        u32    dmaChannel;
        u32    smOffset;
        QueueHandle_t queue;
        SemaphoreHandle_t dmaSemaphore;
    } DHT_Config_t;
#else 
    typedef struct DHT_Config_t {
        u8     gpio;
        u8*    data;
        size_t length;
        vu8    status;
        u8     type;
        QueueHandle_t queue;
    } DHT_Config_t;
#endif

typedef struct DHT_DataPacket_t {
    f32  temperature;
    f32  dewPoint;
    f32  humidity;
    f32  absoluteHumidity;
    u32  timestamp;
    json jsonify;
} DHT_DataPacket_t;

typedef struct DHT_TaskParams_t {
    DHT_Config_t*     dht;
    DHT_DataPacket_t* data;
} DHT_TaskParams_t;


void  DHT_Init(DHT_Config_t* config);
b8    DHT_Read(DHT_Config_t* config);

void DHT_ReadTask(void* pvParameters);


static void DHT_DewPoint(DHT_DataPacket_t* data) {
    f32 gamma = logf(data->humidity / 100.0f) + (17.625f * data->temperature) / (243.04f + data->temperature);
    f32 dewPoint = (243.04f * gamma) / (17.625f - gamma);

    data->dewPoint = dewPoint;
}

static void DHT_AbsoluteHumidity(DHT_DataPacket_t* data) {
    // Magnus-Tetens formula
    f32 pSat = 611.21f * expf((17.67f * data->temperature) / (data->temperature + 243.5f));
    f32 pVapor = pSat * (data->humidity / 100.0f);

    // The constant 2.167 is derived from the molar mass of water and the universal gas constant
    f32 tempKelvin = data->temperature + 273.15f;
    f32 absoluteHumidity = (2.167f * pVapor) / tempKelvin;

    data->absoluteHumidity = absoluteHumidity;
}

static void DHT_ProcessData(DHT_Config_t* config, DHT_DataPacket_t* data) {
    if(config == NULL || data == NULL) {
        HDEBUG("DHT_ProcessData(): DHT was not properly initialized!\n");
        return;
    }

    switch(config->type) {
        case DHT_11: {
            HTRACE("DHT_ProcessData(): Using DHT11 sensor");

            u8 checksum = (config->rawData[0] + config->rawData[1] +config->rawData[2] + config->rawData[3]) & 0xFF; 
            if(checksum != config->rawData[4]) {
                HWARN("DHT_ProcessData(): Data read failed, invalid checksum; Expected: 0x%x ; Got: 0x%x\n", checksum, config->rawData[4]);
                config->status = DHT_READ_BAD_CHECKSUM;
                return;
            }

            data->humidity    =  config->rawData[0] + config->rawData[1] * 0.1f;
            data->temperature =  config->rawData[2] + config->rawData[3] * 0.1f;
            if(config->rawData[2] & 0x80) data->temperature = -data->temperature;
        } break;
        case DHT_22: {
            HTRACE("DHT_ProcessData(): Using DHT22 sensor");

            u16 humidity = (config->rawData[0] << 8) | config->rawData[1];
            u16 temp = (config->rawData[2] << 8) | config->rawData[3];

            f32 humidityRH = (f32)humidity / 10.0f;
            f32 tempC = (f32)(temp & 0x7FFF) / 10.0f;

            if(temp & 0x8000) tempC = -tempC;
            data->humidity = humidityRH;
            data->temperature = tempC;
        } break;   
    }

    DHT_AbsoluteHumidity(data);
    DHT_DewPoint(data);

    // // Magnus-Tetens formula
    // f32 pSat = 611.21f * expf((17.67f * data->temperature) / (data->temperature + 243.5f));
    // f32 pVapor = pSat * (data->humidity / 100.0f);

    // // The constant 216.7 is derived from the molar mass of water and the universal gas constant
    // f32 tempKelvin = data->temperature + 273.15f;
    // f32 absoluteHumidity = (2.167f * pVapor) / tempKelvin;

    // data->absoluteHumidity = absoluteHumidity;


    // f32 gamma = logf(data->humidity/100.0f) + (17.625f * data->temperature) / (243.04f + data->temperature);
    // f32 dewPoint = (243.04f * gamma) / (17.625f - gamma);

    // data->dewPoint = dewPoint;
}

static DHT_DataPacket_t DHT_AverageData(DHT_DataPacket_t* data, size_t len) {
    HTRACE("dht11_22.h -> s:DHT_AverageData(DHT_DataPacket_t*, size_t):DHT_DataPacket_t");

    DHT_DataPacket_t avg = {0};
    if(len < 2) {
        HWARN("DHT_AverageData(): Could not average data from only one sensor");

        avg.humidity    = data->humidity;
        avg.absoluteHumidity = data->absoluteHumidity;
        avg.temperature = data->temperature;
        avg.dewPoint    = data->dewPoint;

        return avg;
    }

    f32 sumRH = 0.0f;
    f32 temp = 0.0f;
    f32 pressure = 0.0f;

    for(size_t i = 0; i < len; ++i) {
        sumRH += data[i].humidity;
        temp  += data[i].temperature;
    }

    avg.humidity    = sumRH/len;
    avg.temperature = temp/len;
    DHT_AbsoluteHumidity(&avg);
    DHT_DewPoint(&avg);

    return avg;
}



static char* DHT_Jsonify(const void* self) {   
    const DHT_DataPacket_t* data = (DHT_DataPacket_t*)self;
    
    const char* json = "{"
        "\"sensor\": \"dht11_22\","
        "\"temperature\": %.2f,"
        "\"humidity\": %.2f,"
        "\"absoluteHumidity\": %.2f"
    "}";
    
    static char buffer[hkDHT_JSON_BUFFER];
    u32 requiredLength = snprintf(buffer, sizeof(buffer), json, data->temperature, data->humidity);
    if(requiredLength > sizeof(buffer)) {
        HERROR("[DHT] Buffer size is to small to send this data packet!\n");
        return NULL;
    }

    return buffer;
}