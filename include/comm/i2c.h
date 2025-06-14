#pragma once
#include <defines.h>
#include <pico/sync.h>

#include <hardware/i2c.h>

typedef struct I2C_Config_t {
    u8  sda;
    u8  scl;
    u32 speed;
    i2c_inst_t* i2c;
    mutex_t* mutex;
} I2C_Config_t;

void I2C_Init(const I2C_Config_t* config);
void I2C_Stop(const I2C_Config_t* config);