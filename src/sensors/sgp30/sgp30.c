#include <core/logger.h>
#include <sensors/sgp30/sgp30.h>

static u8 baselineData[6];


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


i32 SGP30_WriteCommand(I2C_Config_t* i2c, SGP30_Config_t* config, u8 command) {
    HTRACE("sgp30.c -> SGP30_WriteCommand(I2C_Config_t*, SGP30_Config_t*, u8):i32");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("SGP30_WriteCommand(): Mutex acquired");

    const u8 commands[2] = {0x20, command};
    i32 status = i2c_write_blocking(i2c->i2c, hkSGP30_ADDRESS, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("SGP30_WriteCommand(): Couldn't write data to device at address: 0x%x", hkSGP30_ADDRESS);
        mutex_exit(i2c->mutex);

        HTRACE("SGP30_WriteCommand(): Mutex released");
        
        return status;
    } 

    mutex_exit(i2c->mutex);
    HTRACE("SGP30_WriteCommand(): Mutex released");

    return true;
}

i32 SGP30_Init(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_Init(I2C_Config_t*, SGP30_Config_t*):i32");
    HINFO("SGP30 initializing...");

    memset(config->rawData, 0, config->length);
    return SGP30_WriteCommand(i2c, config, hkSGP_INIT);
}

void SGP30_InitRead(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_InitRead(I2C_Config_t*, SGP30_Config_t*):void");

    if(config->status == hkSGP_READ_IN_PROGRESS) return; 
    config->status = hkSGP_READ_IN_PROGRESS;
    
    if(SGP30_WriteCommand(i2c, config, hkSGP_START_MEASURE) == PICO_ERROR_GENERIC) {
        HWARN("SGP30_InitRead(): Could not send start signal to sensor");
        return;
    } 
}

i32 SGP30_Read(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_Read(I2C_Config_t*, SGP30_Config_t*):i32");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("SGP30_Read(): Mutex acquired");

    i32 status = i2c_read_blocking(i2c->i2c, hkSGP30_ADDRESS, config->rawData, config->length, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("SGP30_Read(): Could not start sensor data read");
        mutex_exit(i2c->mutex);
        
        HTRACE("SGP30_Read(): Mutex released");
        
        return status;
    } 

    mutex_exit(i2c->mutex);
    HTRACE("SGP30_Read(): Mutex released");

    config->status = hkSGP_READY;
    return true;
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

void SGP30_InitGetBaseline(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_InitGetBaseline(I2C_Config_t*, SGP30_Config_t*):b8");

    if(config->status == hkSGP_READ_IN_PROGRESS) return; 
    config->status = hkSGP_READ_IN_PROGRESS;
    
    if(SGP30_WriteCommand(i2c, config, hkSGP_GET_BASELINE) == PICO_ERROR_GENERIC) {
        HWARN("SGP30_InitGetBaseline(): Could not init getting baseline from sensor");
        return;
    }
}

i32 SGP30_GetBaseline(I2C_Config_t* i2c, SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_GetBaseline(I2C_Config_t*, SGP30_Config_t*):i32");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("SGP30_GetBaseline(): Mutex acquired");

    i32 status = i2c_read_blocking(i2c->i2c, hkSGP30_ADDRESS, baselineData, sizeof(baselineData), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("SGP30_GetBaseline(): Could not get baseline from sensor");
        mutex_exit(i2c->mutex);

        HTRACE("SGP30_GetBaseline(): Mutex released");
        
        return status;
    } 
    
    mutex_exit(i2c->mutex);
    HTRACE("SGP30_GetBaseline(): Mutex released");
    
    config->status = hkSGP_BASELINE_READY;
    return true;
}

void SGP30_ProcessBaseline(SGP30_Config_t* config) {
    HTRACE("sgp30.c -> SGP30_ProcessBaseline(SGP30_Config_t*):void");

    u16 eco2Baseline = (baselineData[0] << 8) | baselineData[1];
    if(SGP30_CRC(&baselineData[0], 2) != baselineData[2]) {
        HWARN("SGP30_ProcessBaseline(): eCO2 baseline CRC check failed!");
        config->eco2Baseline = 0;
        return;
    }

    u16 tvocBaseline = (baselineData[3] << 8) | baselineData[4];
    if(SGP30_CRC(&baselineData[3], 2) != baselineData[5]) {
        HWARN("SGP30_ProcessBaseline(): TVOC baseline CRC check failed!");
        config->tvocBaseline = 0;
        return;
    }

    config->eco2Baseline = eco2Baseline;
    config->tvocBaseline = tvocBaseline;
} 