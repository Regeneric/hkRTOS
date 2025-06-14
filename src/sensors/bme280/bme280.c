#include <string.h>

#include <core/logger.h>
#include <comm/i2c.h>
#include <sensors/bme280/bme280.h>


// Official Bosch implementation
i32 tFine;
static i32 BME280_CompensateTemp(i32 adcT, BME280_Config_t* config) {
    i32 var1, var2, T;
    var1 = ((((adcT >> 3)  - ((i32) config->params.dig_T1 << 1))) * ((i32) config->params.dig_T2)) >> 11;
    var2 = (((((adcT >> 4) - ((i32) config->params.dig_T1)) * ((adcT >> 4) - ((i32) config->params.dig_T1))) >> 12) * ((i32) config->params.dig_T3)) >> 14;
    tFine = var1 + var2;
    T = (tFine * 5 + 128) >> 8;
    return T;   // Celsius, 0.01 resolution, `2137` equals 21.37 *C`
}

// Official Bosch implementation
static u32 BME280_CompensatePressure(i32 adcP, BME280_Config_t* config) {
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
static u32 BME280_CompensateHumidity(i32 adcH, BME280_Config_t* config) {
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
    HTRACE("BME280_WriteCommands(): Mutex acquired");

    u32 status = i2c_write_blocking(i2c->i2c, hkBME280_ADDRESS, commands, len, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_WriteCommand(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_WriteCommands(): Mutex released");
        
        return false;
    } 
    
    mutex_exit(i2c->mutex);
    HTRACE("BME280_WriteCommands(): Mutex released");

    return status;
}

static i32 BME280_ReadCalibrationData(I2C_Config_t* i2c, BME280_Config_t* config) {
    HTRACE("bme280.c -> s:BME280_ReadCalibrationData(I2C_Config_t*, BME280_Config_t*):i32");
    
    u8 buffer[26];
    u8 commands[1]; commands[0] = BME_REG_CALIB_00;

    // First block of calibration data
    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_ReadCalibrationData(): Mutex acquired");

    i32 status = i2c_write_blocking(i2c->i2c, hkBME280_ADDRESS, commands, 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_ReadCalibrationData(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_ReadCalibrationData(): Mutex released");
        
        return status;
    } 

    status = i2c_read_blocking(i2c->i2c, hkBME280_ADDRESS, buffer, 26, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_ReadCalibrationData(): Could not read data from device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_ReadCalibrationData(): Mutex released");
        
        return status;
    } 

    mutex_exit(i2c->mutex);
    HTRACE("BME280_ReadCalibrationData(): Mutex released");

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
    commands[0] = BME_REG_CALIB_26;

    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_ReadCalibrationData(): Mutex acquired");

    status = i2c_write_blocking(i2c->i2c, hkBME280_ADDRESS, commands, 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_ReadCalibrationData(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_ReadCalibrationData(): Mutex released");
        
        return status;
    }

    status = i2c_read_blocking(i2c->i2c, hkBME280_ADDRESS, buffer, 7, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_ReadCalibrationData(): Could not read data from device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_ReadCalibrationData(): Mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("BME280_ReadCalibrationData(): Mutex released");

    config->params.dig_H2 = (buffer[1] << 8) | buffer[0];
    config->params.dig_H3 =  buffer[2];
    config->params.dig_H4 = (buffer[3] << 4) | (buffer[4] & 0x0F); // H4 and H5 shares the same byte
    config->params.dig_H5 = (buffer[5] << 4) | (buffer[4] >> 4);   // H4 and H5 shares the same byte
    config->params.dig_H6 = (i8)buffer[6];

    return true;    // 1 for success, PICO_ERROR_GENERIC for error
}

i32 BME280_Init(I2C_Config_t* i2c, BME280_Config_t* config) {
    HTRACE("bme280.c -> BME280_Init(I2C_Config_t*, BME280_Config_t*):i32");
    HINFO("BME280 initializing...");

    memset(config->rawData, 0, config->length);

    i32 status = BME280_ReadCalibrationData(i2c, config);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Init(): Could not read calibration data");
        return status;
    } else HINFO("BME280 calibration complete");

    // Humidity
    u8 commands[2];
    commands[0] = BME_REG_CTRL_HUM;
    commands[1] = config->humiditySampling;  // Oversampling x2
    
    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_Init(): Mutex acquired");
    
    status = i2c_write_blocking(i2c->i2c, hkBME280_ADDRESS, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Init(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Init(): Mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("BME280_Init(): Mutex released");

    // IIR filter coefficient
    commands[0] = BME_REG_CONFIG;
    commands[1] = config->iirCoefficient;

    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_Init(): Mutex acquired");

    status = i2c_write_blocking(i2c->i2c, hkBME280_ADDRESS, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Init(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Init(): Mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("BME280_Init(): Mutex released");

    // Pressure and temperature
    commands[0] = BME_REG_CTRL_MEAS;
    commands[1] = config->tempAndPressureMode;

    mutex_enter_blocking(i2c->mutex);
    HTRACE("BME280_Init(): Mutex acquired");

    status = i2c_write_blocking(i2c->i2c, hkBME280_ADDRESS, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Init(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Init(): Mutex released");
        
        return status;
    }
    
    mutex_exit(i2c->mutex);
    HTRACE("BME280_Init(): Mutex released");

    HINFO("BME280 has been configured");
    return true;    // 1 for success, -1 for error
}

i32 BME280_InitRead(I2C_Config_t* i2c, BME280_Config_t* config) {
    HTRACE("bme280.c -> BME280_InitRead(I2C_Config_t*, BME280_Config_t*):i32");

    if(config->status == BME_READ_IN_PROGRESS) return false; 
    config->status = BME_READ_IN_PROGRESS;

    mutex_enter_blocking(i2c->mutex);     
    HTRACE("BME280_InitRead(): Mutex acquired");

    u8 commands[2] = {BME_REG_CTRL_MEAS, 0x49};
    i32 status = i2c_write_blocking(i2c->i2c, hkBME280_ADDRESS, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_InitRead(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_InitRead(): Mutex released");
        
        return status;
    }
    
    mutex_exit(i2c->mutex);
    HTRACE("BME280_InitRead(): Mutex released");

    return true;
}

i32 BME280_Read(I2C_Config_t* i2c, BME280_Config_t* config) {
    HTRACE("bme280.c -> BME280_Read(I2C_Config_t*, BME280_Config_t*):i32");

    mutex_enter_blocking(i2c->mutex); 
    HTRACE("BME280_Read(): Mutex acquired");

    uint8_t reg = 0xF7;
    i32 status = i2c_write_blocking(i2c->i2c, hkBME280_ADDRESS, &reg, 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Read(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Read(): Mutex released");
        
        return status;
    }

    status = i2c_read_blocking(i2c->i2c, hkBME280_ADDRESS, config->rawData, 8, false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("BME280_Read(): Could not write data to device at address: 0x%x", hkBME280_ADDRESS);
        
        mutex_exit(i2c->mutex);
        HTRACE("BME280_Read(): Mutex released");
        
        return status;
    }
    
    mutex_exit(i2c->mutex);
    HTRACE("BME280_Read(): Mutex released");

    config->status = BME_READ_SUCCESS;
    return true;
}

void BME280_ProcessData(BME280_Config_t* config, BME280_DataPacket_t* data) {
    HTRACE("bme280.c -> BME280_ProcessData(BME280_Config_t*, BME280_DataPacket_t*):void");

    i32 rawPressure = (config->rawData[0] << 12) | (config->rawData[1] << 4) | (config->rawData[2] >> 4);
    i32 rawTemp     = (config->rawData[3] << 12) | (config->rawData[4] << 4) | (config->rawData[5] >> 4);
    i32 rawHumidity = (config->rawData[6] <<  8) |  config->rawData[7];

    // IT MUST BE IN THIS ORDER
    i32 temperature = BME280_CompensateTemp(rawTemp, config);
    u32 pressure    = BME280_CompensatePressure(rawPressure, config);
    u32 humidity    = BME280_CompensateHumidity(rawHumidity, config);
    
    data->temperature = temperature / 100.0f;
    data->pressure = (pressure / 256.0f) / 100.0f;;
    data->humidity = humidity / 1024.0f;
}