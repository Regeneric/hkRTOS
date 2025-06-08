#include <config/arm.h>
#if hkEEPROM_24LC01B

#include <stdio.h>
#include <string.h>

#include <hardware/i2c.h>

#include <core/logger.h>
#include <storage/storage.h>
#include <storage/eeprom.h>
#include <comm/i2c.h>


#define EEPROM_ADDR         0x50
#define EEPROM_PAGE_SIZE    8

b8 EEPROM_Write(const void* config, const void* packet) {
    HTRACE("24lc01b.c -> EEPROM_Write(const void*, const void*):b8");

    I2C_Config_t* i2c = (I2C_Config_t*)config;
    const DataPacket_t* data = (DataPacket_t*)packet;
    u8 address = (u8)data->address;

    if(hkEEPROM_24FC01 == false && i2c->speed > 400) {
        HERROR("EEPROM: Clock faster than %u KHz is supported only on 24FC01", i2c->speed);
        return false;
    } 

    // Sanity check
    if(data->size == 0 || data->size > EEPROM_PAGE_SIZE) {
        HDEBUG("EEPROM: Invalid page size %u", data->size);
        return false;
    }

    // Init write
    if(i2c_write_blocking(i2c->i2c, EEPROM_ADDR, &address, 1, true) != 1) {
        HDEBUG("EEPROM: Failed to send control signal to: 0x%x", EEPROM_ADDR);
        return false;
    }

    // Data page write
    u8 buffer[1 + EEPROM_PAGE_SIZE];
    buffer[0] = address;
    memcpy(&buffer[1], data->data, data->size);

    if(i2c_write_blocking(i2c->i2c, EEPROM_ADDR, buffer, (1+data->size), false) != (1+data->size)) {
        HDEBUG("EEPROM: Failed to write data to: 0x%x ; 0x%x", EEPROM_ADDR, address);
        return false;
    }
    
    // Wait for ACK so we're sure that data write is complete
    // while(i2c_write_blocking(i2c->i2c, EEPROM_ADDR, &address, 1, false) != 1);
    sleep_ms(6);
    return true;
}

b8 EEPROM_Read(const void* config, void* packet) {
    HTRACE("24lc01b.c -> EEPROM_Read(const void*, const void*):b8");

    I2C_Config_t* i2c  = (I2C_Config_t*)config;
    DataPacket_t* data = (DataPacket_t*)packet;
    u8 address = (u8)data->address;

    if(hkEEPROM_24FC01 == false && i2c->speed > 400) {
        HERROR("EEPROM: Clock faster than %u KHz is supported only on 24FC01", i2c->speed);
        return false;
    } 

    if(i2c_write_blocking(i2c->i2c, EEPROM_ADDR, &address, 1, true) != 1) {
        HDEBUG("EEPROM: Failed to send control signal to: 0x%x", EEPROM_ADDR);
        return false;
    }

    if(i2c_read_blocking(i2c->i2c, EEPROM_ADDR, data->data, data->size, false) != data->size) {
        HDEBUG("EEPROM: Failed to read data from: 0x%x ; 0x%x", EEPROM_ADDR, address);
        return false;
    } 
    
    return true;
}
#endif