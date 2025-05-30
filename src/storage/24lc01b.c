#include <config/arm.h>
#ifdef hkEEPROM_24LC01B

#include <stdio.h>
#include <string.h>

#include <hardware/i2c.h>

#include <storage/storage.h>
#include <storage/eeprom.h>
#include <comm/i2c.h>


#define EEPROM_ADDR         0x50
#define EEPROM_PAGE_SIZE    8

b8 EEPROM_Write(const void* config, const void* packet) {
    I2C_Config_t* i2c  = (I2C_Config_t*)config;
    DataPacket_t* data = (DataPacket_t*)packet;
    u8 address = (u8)data->address;

    // Sanity check
    if(data->size == 0 || data->size > EEPROM_PAGE_SIZE) {
        printf("EEPROM: Invalid page size %u\n", data->size);
        return FALSE;
    }

    // Init write
    if(i2c_write_blocking(i2c->i2c, EEPROM_ADDR, &address, 1, TRUE) != 1) {
        printf("EEPROM: Failed to send control signal to: 0x%x\n", EEPROM_ADDR);
        return FALSE;
    }

    // Data page write
    u8 buffer[1 + EEPROM_PAGE_SIZE];
    buffer[0] = address;
    memcpy(&buffer[1], data->data, data->size);

    if(i2c_write_blocking(i2c->i2c, EEPROM_ADDR, buffer, (1+data->size), FALSE) != (1+data->size)) {
        printf("EEPROM: Failed to write data to: 0x%x ; 0x%x\n", EEPROM_ADDR, address);
        return FALSE;
    }
    
    // Wait for ACK so we're sure that data write is complete
    // while(i2c_write_blocking(i2c->i2c, EEPROM_ADDR, &address, 1, FALSE) != 1);
    sleep_ms(6);
    return TRUE;
}

b8 EEPROM_Read(const void* config, void* packet) {
    I2C_Config_t* i2c  = (I2C_Config_t*)config;
    DataPacket_t* data = (DataPacket_t*)packet;
    u8 address = (u8)data->address;

    if(i2c_write_blocking(i2c->i2c, EEPROM_ADDR, &address, 1, TRUE) != 1) {
        printf("EEPROM: Failed to send control signal to: 0x%x\n", EEPROM_ADDR);
        return FALSE;
    }

    if(i2c_read_blocking(i2c->i2c, EEPROM_ADDR, data->data, data->size, FALSE) != data->size) {
        printf("EEPROM: Failed to read data from: 0x%x ; 0x%x\n", EEPROM_ADDR, address);
        return FALSE;
    } 
    
    return TRUE;
}
#endif