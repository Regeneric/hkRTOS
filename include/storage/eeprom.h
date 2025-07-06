#pragma once
#include <defines.h>
#include <comm/i2c.h>

b8  EEPROM_Write(const void* config, const void* packet);
b8  EEPROM_Read(const void* config, void* packet);
u16 EEPROM_WriteBlob(const I2C_Config_t* i2c, u16 addr, const u8* data, size_t len);
b8  EEPROM_ReadBlob(const I2C_Config_t* i2c, u16 addr, u8* buff, size_t len);