#include <sensors/dht11_22/dht11_22.h>
#include <sensors/dht20/dht20.h>


static u8 DHT20_CRC(u8* data,  size_t len) {
    u8 crc = 0xFF;
    for(size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for(size_t j = 0; j < 8; j++) {
            if((crc & 0x80) != 0) crc = (crc << 1) ^ 0x31;
            else crc <<= 1;
        }
    } return crc;
}


i32 DHT20_Init(I2C_Config_t* i2c, DHT20_Config_t* config) {
    HTRACE("dht20.c -> DHT20_Init(I2C_Config_t*, DHT20_Config_t*):void");
    HINFO("Initializing DHT20 sensor...");
    
    mutex_enter_blocking(i2c->mutex);
    HTRACE("DHT20_Init(): I2C mutex acquired");

    u8 statusByte = 0;
    i32 status = i2c_read_blocking(i2c->i2c, config->address, &statusByte, 1, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("DHT20_Init(): Could not start sensor status read");
        
        mutex_exit(i2c->mutex);
        HTRACE("DHT20_Init(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("DHT20_Init(): I2C mutex released");

    b8 isCalibrated = (statusByte & 0x08) != 0;
    if(isCalibrated) {
        HINFO("DHT20 sensors is already initialized.");
        return true;
    }

    mutex_enter_blocking(i2c->mutex);
    HTRACE("DHT20_Init(): I2C mutex acquired");

    u8 commands[] = {0xBE, 0x08, 0x00};
    status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("DHT20_Init(): Could not write init commands");
        
        mutex_exit(i2c->mutex);
        HTRACE("DHT20_Init(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("DHT20_Init(): I2C mutex released");

    HINFO("DHT20 sensor has been initalized.");
    return true;
}

i32 DHT20_InitRead(I2C_Config_t* i2c, DHT20_Config_t* config) {
    HTRACE("dht20.c -> DHT20_InitRead(I2C_Config_t*, DHT20_Config_t*):void");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("DHT20_InitRead(): I2C mutex acquired");

    u8 commands[] = {0xAC, 0x33, 0x00};
    i32 status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("DHT20_InitRead(): Could not init read from sensor");
        
        mutex_exit(i2c->mutex);
        HTRACE("DHT20_InitRead(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("DHT20_InitRead(): I2C mutex released");

    return true;
}

i32 DHT20_Read(I2C_Config_t* i2c, DHT20_Config_t* config) {
    HTRACE("dht20.c -> DHT20_Read(I2C_Config_t*, DHT20_Config_t*):void");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("DHT20_Read(): I2C mutex acquired");

    i32 status = i2c_read_blocking(i2c->i2c, config->address, config->rawData, config->length, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("DHT20_Read(): Could not read data from sensor");
        
        mutex_exit(i2c->mutex);
        HTRACE("DHT20_Read(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("DHT20_Read(): I2C mutex released");

    if((config->rawData[0] & 0x80) != 0) {
        HERROR("DHT20_Read(): DHT20 sensor is busy");
        return false;
    }

    u8 checksum = DHT20_CRC(config->rawData, 6);
    if(checksum != config->rawData[6]) {
        HERROR("DHT20_Read(): Invalid CRC. Expected 0x%02X, got 0x%02X\n", checksum, config->rawData[6]);
        return false;
    }

    return true;
}

void DHT20_ProcessData(DHT20_Config_t* config, DHT_DataPacket_t* data) {
    HTRACE("dht20.c -> DHT20_ProcessData(DHT20_Config_t*, DHT_DataPacket_t*):void");

    u32 rawHumidity = ((u32)config->rawData[1] << 12) | ((u32)config->rawData[2] << 4) | (config->rawData[3] >> 4);
    u32 rawTemp     = (((u32)config->rawData[3] & 0x0F) << 16) | ((u32)config->rawData[4] << 8) | config->rawData[5];

    // Convert raw values to final floats using datasheet formulas
    data->humidity    = ((f32)rawHumidity / 1048576.0f) * 100.0f;        // 1048576 = 2^20
    data->temperature = ((f32)rawTemp / 1048576.0f) * 200.0f - 50.0f;
    DHT_AbsoluteHumidity(data);
    DHT_DewPoint(data);
}


void vDHT20_Task(void* pvParameters) {
    HTRACE("dht20.c -> RTOS:vDHT20_Task(void*):void");

    DHT20_TaskParams_t* params = (DHT20_TaskParams_t*)pvParameters;
    UBaseType_t coreID = portGET_CORE_ID();
    
    vTaskDelay(pdMS_TO_TICKS(40));
    while(DHT20_Init(params->i2c, params->dht20) != true) {
        HERROR("vDHT20_Task(): DHT20 failed to initialize! Retrying in 10 seconds...");
        vTaskDelay(pdMS_TO_TICKS(10000));
    }

    vTaskDelay(pdMS_TO_TICKS(10));
    while(FOREVER) {
        HTRACE("vDHT20_Task(): Running on core {%d}", (u16)coreID);

        DHT20_InitRead(params->i2c, params->dht20);
        vTaskDelay(pdMS_TO_TICKS(80));

        if(DHT20_Read(params->i2c, params->dht20)) {
            DHT20_ProcessData(params->dht20, params->data);
            xQueueSend(params->dht20->queue, params->data, 0);
        } else HWARN("vDHT20_Task(): Failed to get valid measurement.");

        vTaskDelay(pdMS_TO_TICKS(1000));    // 1 Hz polling rate
    }
}