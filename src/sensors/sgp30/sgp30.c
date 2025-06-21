#include <core/logger.h>
#include <sensors/sgp30/sgp30.h>

static u8  SGP30_CRC(u8* data, u8 len);
static i32 SGP30_WriteCommand(I2C_Config_t* i2c, SGP30_Config_t* config, u8 command);
static i32 SGP30_CompensateHumidity(I2C_Config_t* i2c, SGP30_Config_t* config, void* sensorData);


static u8 SGP30_CRC(u8* data, u8 len) {
    HTRACE("sgp30.c -> s:SGP30_CRC(u8*, u8):u8");

    u8 crc = 0xFF;
    for(u8 i = 0; i < len; i++) {
        crc ^= data[i];
        for(u8 j = 0; j < 8; j++) {
            if((crc & 0x80) != 0) crc = (u8)((crc << 1) ^ 0x31);
            else crc <<= 1;
        }
    } return crc;
}

static i32 SGP30_WriteCommand(I2C_Config_t* i2c, SGP30_Config_t* config, u8 command) {
    HTRACE("sgp30.c -> s:SGP30_WriteCommand(I2C_Config_t*, SGP30_Config_t*, u8):i32");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("SGP30_WriteCommand(): I2C mutex acquired");

    const u8 commands[2] = {0x20, command};
    i32 status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("SGP30_WriteCommand(): Couldn't write data to device at address: 0x%x", config->address);
        mutex_exit(i2c->mutex);

        HTRACE("SGP30_WriteCommand(): I2C mutex released");
        
        return status;
    } 

    mutex_exit(i2c->mutex);
    HTRACE("SGP30_WriteCommand(): I2C mutex released");

    return true;
}



i32 SGP30_Init(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_Init(I2C_Config_t*, SGP30_Config_t*):i32");
    HINFO("SGP30 initializing...");

    memset(config->rawData, 0, config->length);
    return SGP30_WriteCommand(i2c, config, SGP30_INIT);
}


void SGP30_InitRead(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_InitRead(I2C_Config_t*, SGP30_Config_t*):void");

    if(config->status == SGP30_READ_IN_PROGRESS) return; 
    config->status = SGP30_READ_IN_PROGRESS;
    
    if(SGP30_WriteCommand(i2c, config, SGP30_START_MEASURE) == PICO_ERROR_GENERIC) {
        HWARN("SGP30_InitRead(): Could not send start signal to sensor");
        return;
    } 
}

i32 SGP30_Read(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_Read(I2C_Config_t*, SGP30_Config_t*):i32");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("SGP30_Read(): I2C mutex acquired");

    i32 status = i2c_read_blocking(i2c->i2c, config->address, config->rawData, config->length, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("SGP30_Read(): Could not start sensor data read");
        mutex_exit(i2c->mutex);
        
        HTRACE("SGP30_Read(): I2C mutex released");
        
        return status;
    } 

    mutex_exit(i2c->mutex);
    HTRACE("SGP30_Read(): I2C mutex released");

    config->status = SGP30_READY;
    return true;
}

i32 SGP30_ReadBlocking(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_ReadBlocking(I2C_Config_t*, SGP30_Config_t*):i32");

    SGP30_InitRead(i2c, config);
    sleep_ms(15);
    return SGP30_Read(i2c, config);
}


void SGP30_ProcessData(SGP30_Config_t* config, SGP30_DataPacket_t* data) {
    HTRACE("sgp30.c -> SGP30_ProcessData(SGP30_Config_t*, SGP30_DataPacket_t*):void");

    u16 eco2 = (config->rawData[0] << 8) | config->rawData[1];
    if(SGP30_CRC(&config->rawData[0], 2) != config->rawData[2]) {
        HWARN("SGP30_ProcessData(): eCO2 CRC check failed!");
        return;
    }

    u16 tvoc = (config->rawData[3] << 8) | config->rawData[4];
    if(SGP30_CRC(&config->rawData[3], 2) != config->rawData[5]) {
        HWARN("SGP30_ProcessData(): TVOC CRC check failed!");
        return;
    }

    data->eco2 = eco2;
    data->tvoc = tvoc;
}

#if hkSGP30_USE_BME280 && hkSGP_USE_DHT
    #error "SGP30: You can use either DHT or BME280 for humidity compensation, not both"
#endif

#if hkSGP30_USE_SENSOR && hkSGP30_HUMID_COMPENSATION
#include <math.h>
#include <sensors/bme280/bme280.h>
#include <sensors/dht11_22/dht11_22.h>

static i32 SGP30_CompensateHumidity(I2C_Config_t* i2c, SGP30_Config_t* config, void* sensorData) {
    HTRACE("sgp30.c -> s:SGP30_CompensateHumidity(SGP30_DataPacket_t*, void*):i32");

    #if hkSGP30_USE_BME280 && !hkSGP_USE_DHT
        BME280_DataPacket_t* data = (BME280_DataPacket_t*)sensorData; 
    #elif !hkSGP30_USE_BME280 && hkSGP_USE_DHT
        DHT_DataPacket_t* data = (DHT_DataPacket_t*)sensorData; 
    #elif hkSGP30_USE_BME280 && hkSGP_USE_DHT
        HWARN("sgp30(): Both hkSGP30_USE_BME280 and hkSGP_USE_DHT are set to 'true'. Data will not be compensated");
        return false;
    #else
        HWARN("sgp30(): Both hkSGP30_USE_BME280 and hkSGP_USE_DHT are set to 'false'. Data will not be compensated");
        return false;
    #endif

    // 8.8 format
    u16 humidCompensation = (u16)(data->absoluteHumidity * 256.0f);

    mutex_enter_blocking(i2c->mutex);
    HTRACE("SGP30_CompensateHumidity(): I2C mutex acquired");

    u8 humidMSB = (humidCompensation >> 8) & 0xFF;
    u8 humidLSB =  humidCompensation & 0xFF;

    u8 commands[5] = {0x20, SGP30_SET_HUMIDITY, humidMSB, humidLSB};
    commands[4] = SGP30_CRC(&commands[2], 2);

    i32 status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("SGP30_CompensateHumidity(): Couldn't write data to device at address: 0x%x", config->address);
        mutex_exit(i2c->mutex);

        HTRACE("SGP30_CompensateHumidity(): I2C mutex released");
        
        return status;
    } 

    mutex_exit(i2c->mutex);
    HTRACE("SGP30_CompensateHumidity(): I2C mutex released");

    return true;
}



void SGP30_InitReadHumidCompensation(I2C_Config_t* i2c, SGP30_Config_t* config, void* sensorData) {
    HTRACE("sgp30.c -> SGP30_InitReadHumidCompensation(I2C_Config_t*, SGP30_Config_t*, void*):void");

    if(config->status == SGP30_READ_IN_PROGRESS) return; 
    config->status = SGP30_READ_IN_PROGRESS;
    
    SGP30_CompensateHumidity(i2c, config, sensorData);
    if(SGP30_WriteCommand(i2c, config, SGP30_START_MEASURE) == PICO_ERROR_GENERIC) {
        HWARN("SGP30_InitReadHumidCompensation(): Could not send start signal to sensor");
        return;
    } 
}

i32 SGP30_ReadBlockingHumidComp(I2C_Config_t* i2c, SGP30_Config_t* config, void* sensorData) {
    HTRACE("sgp30.c -> SGP30_ReadBlockingHumidComp(I2C_Config_t*, SGP30_Config_t*):i32");

    SGP30_InitReadHumidCompensation(i2c, config, sensorData);
    sleep_ms(15);
    return SGP30_Read(i2c, config);
}
#endif


void vSGP30_Task(void* pvParameters) {
    HTRACE("sgp30.c -> vSGP30_Task(void*):void");

    SGP30_TaskParams_t* params = (SGP30_TaskParams_t*)pvParameters;
    UBaseType_t coreID = portGET_CORE_ID();

    // Error loop
    while(SGP30_Init(params->i2c, params->sgp30) != true) {
        HFATAL("vSGP30_Task(): SGP30 failed to initialize! Retrying in 10 seconds...");
        vTaskDelay(pdMS_TO_TICKS(10000));
    } 

    vTaskDelay(pdMS_TO_TICKS(100));
    while(FOREVER) {
        HTRACE("vSGP30_Task(): Running on core {%d}", (u16)coreID);

        #if hkSGP30_USE_SENSOR && hkSGP30_HUMID_COMPENSATION
            SGP30_InitReadHumidCompensation(params->i2c, params->sgp30, params->humidSensor);
        #else
            SGP30_InitRead(params->i2c, params->sgp30);
        #endif

        vTaskDelay(pdMS_TO_TICKS(15));
        SGP30_Read(params->i2c, params->sgp30);

        if(params->sgp30->status == SGP30_READY) {
            SGP30_ProcessData(params->sgp30, params->data);
            xQueueSend(params->sgp30->queue, params->data, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));    // 1 Hz polling rate
    }
}