#include <core/logger.h>
#include <sensors/pms5003/pms5003.h>

#if hkPMS5003_USE_BME280 && hkPMS5003_USE_DHT
    #error "PMS5003: You can use either DHT or BME280 for humidity compensation, not both"
#endif

#if hkBME280_USE_SENSOR && hkPMS5003_HUMID_COMPENSATION
#include <math.h>
#include <sensors/bme280/bme280.h>
#include <sensors/dht11_22/dht11_22.h>

static void PMS5003_CompensateHumidity(PMS5003_DataPacket_t* pmsData, void* sensorData) {
    HTRACE("pms5003.c -> s:PMS5003_CompensateHumidity(PMS5003_Config_t*, void*):void");

    #if hkPMS5003_USE_BME280 && !hkPMS5003_USE_DHT
        BME280_DataPacket_t* data = (BME280_DataPacket_t*)sensorData; 
    #elif hkPMS5003_USE_DHT && !hkPMS5003_USE_BME280
        DHT_DataPacket_t* data = (DHT_DataPacket_t*)sensorData; 
    #elif hkPMS5003_USE_BME280 && hkPMS5003_USE_DHT
        HWARN("PMS5003_CompensateHumidity(): Bot hkPMS5003_USE_BME280 and hkPMS5003_USE_DHT are set to 'true'. Data will not be compensated");
        return;
    #else
        HWARN("PMS5003_CompensateHumidity(): Bot hkPMS5003_USE_BME280 and hkPMS5003_USE_DHT are set to 'false'. Data will not be compensated");
        return;
    #endif

    // We can try to "dry" our particles
    f32 kappa = 0.25f; 
    // TODO: i need real time clock
    // Winter-autumn: K ~ 0.05-0.10
    // Spring-summer: K ~ 0.15-0.25
    // switch(month) {
    //     case 12:  case 1:  case 2: kappa = 0.08f; break;
    //     case  3:  case 4:  case 5: kappa = 0.13f; break;
    //     case  6:  case 7:  case 8: kappa = 0.25f; break;
    //     case  9: case 10: case 11: kappa = 0.15f; break;
    //     default: kappa = 0.20f; break; 
    // }

    b8 pm10Comp = true;
    if(data->humidity <= 10.0f) {
        HTRACE("PMS5003_CompensateHumidity(): Humidity is less than 10 %RH. Data will not be compensated");
        return;
    }
    if(data->humidity <= 50.0f) {
        HTRACE("PMS5003_CompensateHumidity(): Humidity is less than 50 %RH. PM10 data will not be compensated");
        pm10Comp = false;
    }
    if(data->humidity >= 85.0f) {
        HTRACE("PMS5003_CompensateHumidity(): Humidity is less than 85 %RH. PM10 data will not be compensated");
        pm10Comp = false;
    }
    if(data->humidity >= 90.0f) {
        HTRACE("PMS5003_CompensateHumidity(): Humidity is more than 90 %RH. Data will not be compensated");
        return;
    }

    if(pm10Comp) {
        f32 rh = data->humidity / 100.0f;
        f32 gf = powf(1 + kappa * (rh / (1 - rh)), 1.0f/3.0f);

        f32 pm10 = (f32)((f32)pmsData->pm10 / gf);
        pmsData->pm10  = (u16)(pm10 + 0.5f);
    }

    // Correction values based on https://www.mdpi.com/1424-8220/22/24/9669 and https://www.epa.gov/system/files/documents/2024-06/particulate-matter-pm2.5-sensor-loan-program-qapp-aasb-qapp-004-r1.1.pdf
    f32 pm1 = (f32)(0.524f * (f32)pmsData->pm1) - (0.0862f * data->humidity) + (0.0753f * data->temperature) + 5.75f;
    pmsData->pm1 = (u16)(pm1 + 0.5f); 

    f32 pm2_5 = (f32)(0.524f * (f32)pmsData->pm2_5) - (0.0862f * data->humidity) + (0.0753f * data->temperature) + 5.75f;
    pmsData->pm2_5 = (u16)(pm2_5 + 0.5f);
}
#endif


static b8 PMS5003_ValidatePacket(PMS5003_Config_t* config) {
    HTRACE("pms5003.c -> s:PMS5003_ValidatePacket(PMS5003_Config_t*):b8");

    if(config->rawData[0] != 0x42 || config->rawData[1] != 0x4D) {
        HWARN("PMS5003_ValidatePacket(): Validation failed; invalid start bytes.");
        return false;
    }

    u16 checksum = 0;
    for(u8 i = 0; i < (PMS5003_PACKET_LENGTH-2); i++) checksum += config->rawData[i];

    u16 sensorChecksum = (config->rawData[30] << 8) | config->rawData[31];
    return (checksum == sensorChecksum);
}

void PMS5003_ProcessData(PMS5003_Config_t* config, PMS5003_DataPacket_t* data) {
    HTRACE("pms5003.c -> PMS5003_ProcessData(PMS5003_Config_t*, PMS5003_DataPacket_t*):void");
    if(!PMS5003_ValidatePacket(config)) return;

    data->pm1   = (config->rawData[4] << 8) | config->rawData[5];
    data->pm2_5 = (config->rawData[6] << 8) | config->rawData[7];
    data->pm10  = (config->rawData[8] << 8) | config->rawData[9];
}

void PMS5003_ProcessDataHumidCompensation(PMS5003_Config_t* config, PMS5003_DataPacket_t* data, void* humidityData) {
    HTRACE("pms5003.c -> PMS5003_ProcessDataHumidCompensation(PMS5003_Config_t*, PMS5003_DataPacket_t*, void*):void");
    if(!PMS5003_ValidatePacket(config)) return;

    PMS5003_ProcessData(config, data);

    #if hkBME280_USE_SENSOR && hkPMS5003_HUMID_COMPENSATION && hkPMS5003_USE_BME280
        BME280_DataPacket_t* bmeData = (BME280_DataPacket_t*)humidityData;
        PMS5003_CompensateHumidity(data, bmeData);
    #elif hkDHT_USE_SENSOR && hkPMS5003_HUMID_COMPENSATION && hkPMS5003_USE_DHT
        DHT_DataPacket_t* dhtData = (DHT_DataPacket_t*)humidityData;
        PMS5003_CompensateHumidity(data, dhtData);
    #else
        HWARN("PMS5003_ProcessDataHumidCompensation(): Bot hkPMS5003_USE_BME280 and hkPMS5003_USE_DHT are set to 'false'. Data will not be compensated");
    #endif
}

b8 PMS5003_Read(UART_Config_t* uart, PMS5003_Config_t* config) {
    HTRACE("pms5003.c -> PMS5003_Read(UART_Config_t*, PMS5003_Config_t*):b8");

    UART_ReadPacket(uart);
    config->rawData = uart->data;
    return true;
}


void vPMS5003_Task(void* pvParameters) {
    HTRACE("pms5003.c -> RTOS:vPMS5003_Task(void*):void");

    PMS5003_TaskParams_t* params = (PMS5003_TaskParams_t*)pvParameters;
    UBaseType_t coreID = portGET_CORE_ID();

    vTaskDelay(pdMS_TO_TICKS(100));
    while(FOREVER) {
        HTRACE("vPMS5003_Task(): Running on core {%d}", (u16)coreID);
    
        PMS5003_Read(params->uart, params->pms5003);

        if(xSemaphoreTake(params->uart->dmaSemaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
            #if hkBME280_USE_SENSOR && hkPMS5003_HUMID_COMPENSATION
                PMS5003_ProcessDataHumidCompensation(params->pms5003, params->data, params->humidSensor);
            #else
                PMS5003_ProcessData(params->pms5003, params->data);
            #endif

            xQueueSend(params->pms5003->queue, params->data, 0);
        } else HWARN("vPMS5003_Task(): Timed out waiting for UART DMA completion.");

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}