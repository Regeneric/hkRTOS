#include <config/arm.h>
#if hkOLED_SH1107

#include <string.h>

#include <hardware/i2c.h>

#include <core/gpio.h>

#include <display/display.h>
#include <display/sh1107/sh1107_config.h>

static I2C_Config_t* sgI2C;

b8 Display_Init(I2C_Config_t* i2c, DisplayConfig_t* config) {
    HTRACE("sh1107.c -> Display_Init(I2C_Config_t*, SSH1107_Config_t*):b8");
    
    gpio_init(10);
    gpio_set_dir(10, GPIO_OUT);
    gpio_put(10, GPIO_HIGH);
    sleep_ms(1);
    gpio_put(10, GPIO_LOW);
    sleep_ms(1);
    gpio_put(10, GPIO_HIGH);

    sgI2C = i2c;
    
    static const u8 initCommands[] = {
        SH1107_DISPLAYOFF,
        SH1107_DISPLAYCLOCKDIV,  0x51,
        SH1107_MEMORYMODE,
        SH1107_CONTRAST,         0x4F,
        SH1107_DCDC,             0x8A,
        SH1107_SEGREMAP,
        SH1107_COMSCANINC,
        SH1107_DISPLAYSTARTLINE, 0x00,
        SH1107_DISPLAYOFFSET,    0x60,
        SH1107_PRECHARGE,        0x22,
        SH1107_VCOMDETECT,       0x35,
        SH1107_MULTIPLEX,        0x3F,
        SH1107_DISPLAYON_RESUME ,
        SH1107_NORMALDISPLAY

    }; Display_WriteCommandList(initCommands, sizeof(initCommands));
    
    if(config->width == 128 && config->height == 128) {
        HDEBUG("SH1107_Init(): Display set to 128x128");
        static const u8 override[] = {
            SH1107_DISPLAYOFFSET, 0x00,
            SH1107_MULTIPLEX,     0x7F
        }; Display_WriteCommandList(override, sizeof(override));
    }

    sleep_ms(100);
    Display_WriteCommand(SH1107_DISPLAYON);
}

u32 Display_WriteCommand(u8 command) {
    HTRACE("sh1107.c -> Display_WriteCommand(I2C_Config_t*, u8):b8");

    if(sgI2C == NULL) {
        HWARN("Display_WriteCommand(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[2];
    buffer[0] = 0x00;   // Control Byte: the following data is a command
    buffer[1] = command;

    u32 status = i2c_write_blocking(sgI2C->i2c, SH1107_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("Display_WriteCommand(): Couldn't write data to device at address: 0x%x", SH1107_ADDRESS);
        return false;
    } else return status;
}

u32 Display_WriteCommandList(const u8* commands, size_t len) {
    HTRACE("sh1107.c -> Display_WriteCommandList(I2C_Config_t*, u8*, size_t):b8");

    if(sgI2C == NULL) {
        HWARN("Display_WriteCommandList(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[len+1];
    buffer[0] = 0x00;   // Control Byte: the following data is a command
    memcpy(buffer+1, commands, len);

    u32 status = i2c_write_blocking(sgI2C->i2c, SH1107_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("Display_WriteCommandList(): Couldn't write data to device at address: 0x%x", SH1107_ADDRESS);
        return false;
    } else return status;
}

u32 Display_WriteData(const u8* data, size_t len) {
    HTRACE("sh1107.c -> Display_WriteData(I2C_Config_t*, u8*, size_t):b8");

    if(sgI2C == NULL) {
        HWARN("SH1107_WriteCommandList(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[len+1];
    buffer[0] = 0x40;   // Control Byte: the following data is a pixel
    memcpy(buffer+1, data, len);

    u32 status = i2c_write_blocking(sgI2C->i2c, SH1107_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("Display_WriteData(): Couldn't write data to device at address: 0x%x", SH1107_ADDRESS);
        return false;
    } else return status;
}

#endif