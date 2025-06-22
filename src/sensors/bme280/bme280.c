#include <string.h>
#include <math.h>

#include <core/logger.h>
#include <sensors/bme280/bme280.h>


// Official Bosch implementation
static i32 BME280_CompensateTemp(i32 adcT, BME280_Config_t* config, i32* tFine) {
    i32 var1, var2, T;
    var1 = ((((adcT >> 3)  - ((i32) config->params.dig_T1 << 1))) * ((i32) config->params.dig_T2)) >> 11;
    var2 = (((((adcT >> 4) - ((i32) config->params.dig_T1)) * ((adcT >> 4) - ((i32) config->params.dig_T1))) >> 12) * ((i32) config->params.dig_T3)) >> 14;
    *tFine = var1 + var2;
    T = (*tFine * 5 + 128) >> 8;
    return T;   // Celsius, 0.01 resolution, `2137` equals 21.37 *C`
}

// Official Bosch implementation
static u32 BME280_CompensatePressure(i32 adcP, BME280_Config_t* config, i32 tFine) {
    i64 var1, var2, p;
    var1 = ((i64) tFine) - 128000;
    var2 = var1 * var1 * (i64)config->params.dig_P6;
    var2 = var2 + ((var1 * (i64)config->params.dig_P5) << 17);
    var2 = var2 + (((i64)config->params.dig_P4) << 35);
    var1 = ((var1 * var1 * (i64)config->params.dig_P3) >> 8) + ((var1 * (i64)config->params.dig_P2) << 12);
    var1 = (((((i64)1) << 47) + var1)) * ((i64)config->params.dig_P1) >> 33;
    if(var1 == 0) return 0; // avoid exception caused by division by zero

    p = 1048576 - adcP;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((i64)config->params.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((i64)config->params.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((i64)config->params.dig_P7) << 4);
    return (u32)p;  // Q24.8 format, `24674867` equals to `24674867/256 == 96386.2 Pa == 963.862 hPa`
}

// Official Bosch implementation
static u32 BME280_CompensateHumidity(i32 adcH, BME280_Config_t* config, i32 tFine) {
    i32 v_x1_u32r;
    v_x1_u32r = (tFine - ((i32)76800));

    // What the fuck is going on here???
    v_x1_u32r = (((((adcH << 14) - (((i32)config->params.dig_H4) << 20) - (((i32)config->params.dig_H5) * v_x1_u32r)) +
                    ((i32)16384)) >> 15) * (((((((v_x1_u32r * ((i32)config->params.dig_H6)) >> 10) *
                    (((v_x1_u32r * ((i32)config->params.dig_H3)) >> 11) +
                    ((i32)32768))) >> 10) + ((i32)2097152)) *
                    ((i32)config->params.dig_H2) + 8192) >> 14));

    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((i32)config->params.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return (u32)(v_x1_u32r >> 12);  // Q22.10 format, `47445` equals to `47445/1024 == 46.333 %RH`
}


i32 BME280_WriteCommands(I2C_Config_t* i2c, BME280_Config_t* config, u8* commands, size_t len) {
    HTRACE("bme280.c -> BME280_WriteCommand(I2C_Config_t*, BME280_Config_t*, u8*, size_t):i32");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_WriteCommands(): I2C mutex acquired");

    u32 status = i2c_write_blocking(i2c->i2c, config->address, commands, len, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_WriteCommand(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_WriteCommands(): I2C mutex released");
        
        return false;
    } 
    
    mutex_exit(i2c->mutex);
    HTRACE("BME280_WriteCommands(): I2C mutex released");

    return status;
}

static i32 BME280_ReadCalibrationData(I2C_Config_t* i2c, BME280_Config_t* config) {
    HTRACE("bme280.c -> s:BME280_ReadCalibrationData(I2C_Config_t*, BME280_Config_t*):i32");
    
    u8 buffer[26];
    u8 commands[1]; commands[0] = BME280_REG_CALIB_00;

    // First block of calibration data
    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_ReadCalibrationData(): I2C mutex acquired");

    i32 status = i2c_write_blocking(i2c->i2c, config->address, commands, 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_ReadCalibrationData(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_ReadCalibrationData(): I2C mutex released");
        
        return status;
    } 

    status = i2c_read_blocking(i2c->i2c, config->address, buffer, 26, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_ReadCalibrationData(): Could not read data from device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_ReadCalibrationData(): I2C mutex released");
        
        return status;
    } 

    mutex_exit(i2c->mutex);
    HTRACE("BME280_ReadCalibrationData(): I2C mutex released");

    config->params.dig_T1 = (buffer[1]  << 8) | buffer[0];
    config->params.dig_T2 = (buffer[3]  << 8) | buffer[2];
    config->params.dig_T3 = (buffer[5]  << 8) | buffer[4];

    config->params.dig_P1 = (buffer[7]  << 8) | buffer[6];
    config->params.dig_P2 = (buffer[9]  << 8) | buffer[8];
    config->params.dig_P3 = (buffer[11] << 8) | buffer[10];
    config->params.dig_P4 = (buffer[13] << 8) | buffer[12];
    config->params.dig_P5 = (buffer[15] << 8) | buffer[14];
    config->params.dig_P6 = (buffer[17] << 8) | buffer[16];
    config->params.dig_P7 = (buffer[19] << 8) | buffer[18];
    config->params.dig_P8 = (buffer[21] << 8) | buffer[20];
    config->params.dig_P9 = (buffer[23] << 8) | buffer[22];

    // Second block of calibration data
    config->params.dig_H1 = buffer[25];
    commands[0] = BME280_REG_CALIB_26;

    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_ReadCalibrationData(): I2C mutex acquired");

    status = i2c_write_blocking(i2c->i2c, config->address, commands, 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_ReadCalibrationData(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_ReadCalibrationData(): I2C mutex released");
        
        return status;
    }

    status = i2c_read_blocking(i2c->i2c, config->address, buffer, 7, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_ReadCalibrationData(): Could not read data from device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_ReadCalibrationData(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("BME280_ReadCalibrationData(): I2C mutex released");

    config->params.dig_H2 = (buffer[1] << 8) | buffer[0];
    config->params.dig_H3 =  buffer[2];
    config->params.dig_H4 = (buffer[3] << 4) | (buffer[4] & 0x0F); // H4 and H5 shares the same byte
    config->params.dig_H5 = (buffer[5] << 4) | (buffer[4] >> 4);   // H4 and H5 shares the same byte
    config->params.dig_H6 = (i8)buffer[6];

    return true;    // 1 for success, PICO_ERROR_GENERIC for error
}

i32 BME280_Init(I2C_Config_t* i2c, BME280_Config_t* config) {
    HTRACE("bme280.c -> BME280_Init(I2C_Config_t*, BME280_Config_t*):i32");
    HINFO("Initializing BME280 sensor...");
    HINFO("Calibrating BME280 sensor...");

    memset(config->rawData, 0, config->length);

    i32 status = BME280_ReadCalibrationData(i2c, config);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Init(): Could not read calibration data");
        return status;
    } else HINFO("BME280 sensor has been calibrated.");

    // Humidity
    u8 commands[2];
    commands[0] = BME280_REG_CTRL_HUM;
    commands[1] = config->humiditySampling;  // Oversampling x2
    
    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_Init(): I2C mutex acquired");
    
    status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Init(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Init(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("BME280_Init(): I2C mutex released");

    // IIR filter coefficient
    commands[0] = BME280_REG_CONFIG;
    commands[1] = config->iirCoefficient;

    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_Init(): I2C mutex acquired");

    status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Init(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Init(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("BME280_Init(): I2C mutex released");

    // Pressure and temperature
    commands[0] = BME280_REG_CTRL_MEAS;
    commands[1] = config->tempAndPressureMode;

    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_Init(): I2C mutex acquired");

    status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Init(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Init(): I2C mutex released");
        
        return status;
    }
    
    mutex_exit(i2c->mutex);
    HTRACE("BME280_Init(): I2C mutex released");

    HINFO("BME280 sensor has been initialized.");
    return true;    // 1 for success, -1 for error
}

i32 BME280_InitRead(I2C_Config_t* i2c, BME280_Config_t* config) {
    HTRACE("bme280.c -> BME280_InitRead(I2C_Config_t*, BME280_Config_t*):i32");

    if(config->status == BME280_READ_IN_PROGRESS) return false; 
    config->status = BME280_READ_IN_PROGRESS;

    u8 commands[2];
    commands[0] = BME280_REG_CTRL_HUM;
    commands[1] = config->humiditySampling;

    mutex_enter_blocking(i2c->mutex);     
    HTRACE("BME280_InitRead(): I2C mutex acquired");

    i32 status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_InitRead(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_InitRead(): I2C mutex released");
        
        return status;
    }
    
    mutex_exit(i2c->mutex);
    HTRACE("BME280_InitRead(): I2C mutex released");

    commands[0] = BME280_REG_CTRL_MEAS;
    commands[1] = config->tempAndPressureMode;

    mutex_enter_blocking(i2c->mutex);     
    HTRACE("BME280_InitRead(): I2C mutex acquired");

    status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_InitRead(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_InitRead(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("BME280_InitRead(): I2C mutex released");

    return true;
}

i32 BME280_Read(I2C_Config_t* i2c, BME280_Config_t* config) {
    HTRACE("bme280.c -> BME280_Read(I2C_Config_t*, BME280_Config_t*):i32");

    mutex_enter_blocking(i2c->mutex); 
    HTRACE("BME280_Read(): I2C mutex acquired");

    uint8_t reg = 0xF7;
    i32 status = i2c_write_blocking(i2c->i2c, config->address, &reg, 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Read(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Read(): I2C mutex released");
        
        return status;
    }

    status = i2c_read_blocking(i2c->i2c, config->address, config->rawData, 8, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Read(): Could not write data to device at address: 0x%x", config->address);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Read(): I2C mutex released");
        
        return status;
    }
    
    mutex_exit(i2c->mutex);
    HTRACE("BME280_Read(): I2C mutex released");

    config->status = BME280_READ_SUCCESS;
    return true;
}


static void BME280_DewPoint(BME280_DataPacket_t* data) {
    f32 gamma = logf(data->humidity / 100.0f) + (17.625f * data->temperature) / (243.04f + data->temperature);
    f32 dewPoint = (243.04f * gamma) / (17.625f - gamma);

    data->dewPoint = dewPoint;
}

static void BME280_AbsoluteHumidity(BME280_DataPacket_t* data) {
    // Magnus-Tetens formula
    f32 pSat = 611.21f * expf((17.67f * data->temperature) / (data->temperature + 243.5f));
    f32 pVapor = pSat * (data->humidity / 100.0f);

    // The constant 2.167 is derived from the molar mass of water and the universal gas constant
    f32 tempKelvin = data->temperature + 273.15f;
    f32 absoluteHumidity = (2.167f * pVapor) / tempKelvin;

    data->absoluteHumidity = absoluteHumidity;
}

void BME280_ProcessData(BME280_Config_t* config, BME280_DataPacket_t* data) {
    HTRACE("bme280.c -> BME280_ProcessData(BME280_Config_t*, BME280_DataPacket_t*):void");

    i32 rawPressure = (config->rawData[0] << 12) | (config->rawData[1] << 4) | (config->rawData[2] >> 4);
    i32 rawTemp     = (config->rawData[3] << 12) | (config->rawData[4] << 4) | (config->rawData[5] >> 4);
    i32 rawHumidity = (config->rawData[6] <<  8) |  config->rawData[7];

    // IT MUST BE IN THIS ORDER
    i32 tFine = 0;
    i32 temperature = BME280_CompensateTemp(rawTemp, config, &tFine);
    u32 pressure    = BME280_CompensatePressure(rawPressure, config, tFine);
    u32 humidity    = BME280_CompensateHumidity(rawHumidity, config, tFine);

    data->temperature = temperature / 100.0f;
    data->pressure = (pressure / 256.0f) / 100.0f;;
    data->humidity = humidity / 1024.0f;
    BME280_AbsoluteHumidity(data);
    BME280_DewPoint(data);
}

BME280_DataPacket_t BME280_AverageData(BME280_DataPacket_t* data, size_t len) {
    HTRACE("bme280.c -> BME280_AverageData(BME280_DataPacket_t*, size_t):BME280_DataPacket_t");

    BME280_DataPacket_t avg = {0};
    if(len < 2) {
        HWARN("BME280_AverageData(): Could not average data from only one sensor");

        avg.humidity    = data->humidity;
        avg.absoluteHumidity = data->absoluteHumidity;
        avg.temperature = data->temperature;
        avg.dewPoint    = data->dewPoint;
        avg.pressure    = data->pressure;

        return avg;
    }

    f32 sumRH = 0.0f;
    f32 temp = 0.0f;
    f32 pressure = 0.0f;

    for(size_t i = 0; i < len; ++i) {
        sumRH += data[i].humidity;
        temp  += data[i].temperature;
        pressure += data[i].pressure;
    }

    avg.humidity    = sumRH/len;
    avg.temperature = temp/len;
    avg.pressure    = pressure/len;
    BME280_AbsoluteHumidity(&avg);
    BME280_DewPoint(&avg);

    return avg;
}


void vBME280_Task(void* pvParameters) {
    HTRACE("bme280.c -> RTOS:vBME280_Task(void*):void");

    BME280_TaskParams_t* params = (BME280_TaskParams_t*)pvParameters;
    UBaseType_t coreID = portGET_CORE_ID();

    // Error loop
    while(BME280_Init(params->i2c, params->bme280) != true) {
        HERROR("vBME280_Task(): BME280 failed to initialize! Retrying in 10 seconds...");
        vTaskDelay(pdMS_TO_TICKS(10000));
    } 
    
    vTaskDelay(pdMS_TO_TICKS(100));
    while(FOREVER) {
        HTRACE("vBME280_Task(): Running on core {%d}", (u16)coreID);

        BME280_InitRead(params->i2c, params->bme280);
        vTaskDelay(pdMS_TO_TICKS(15));
        BME280_Read(params->i2c, params->bme280);

        if(params->bme280->status == BME280_READ_SUCCESS) {
            BME280_ProcessData(params->bme280, params->data);
            xQueueSend(params->bme280->queue, params->data, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));    // 1 Hz polling rate
    }
}