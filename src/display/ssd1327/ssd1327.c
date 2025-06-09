#include <config/arm.h>
#if hkOLED_SSD1327

#include <string.h>

#include <hardware/i2c.h>

#include <core/gpio.h>

#include <display/display.h>
#include <display/ssd1327/ssd1327_config.h>

static I2C_Config_t* sgI2C;

b8 Display_Init(I2C_Config_t* i2c, DisplayConfig_t* config) {
    HTRACE("ssd1327.c -> Display_Init(I2C_Config_t*, SSH1107_Config_t*):b8");
    
    // TODO: reset function
    gpio_init(10);
    gpio_set_dir(10, GPIO_OUT);
    gpio_put(10, GPIO_HIGH);
    sleep_ms(1);
    gpio_put(10, GPIO_LOW);
    sleep_ms(1);
    gpio_put(10, GPIO_HIGH);

    sgI2C = i2c;
    
    static const u8 initCommands[] = {
        // SSD1327_DISPLAY_OFF,
        // SSD1327_SET_COMMAND_LOCK,       0x12,
        // SSD1327_SET_DISPLAY_OFFSET,     0x00,
        // SSD1327_SET_REMAP,              0x51,
        // SSD1327_SET_FUNC_SELECT_A,      0x01,
        // SSD1327_SET_PHASE_LENGTH,       0x51,
        // SSD1327_SET_CLOCK_DIV,          0x00,
        // SSD1327_SET_PRECHARGE,          0x08,
        // SSD1327_SET_VCOMH,              0x0F,
        // SSD1327_SET_SECOND_PRECHARGE,   0x04,
        // SSD1327_SET_FUNC_SELECT_B,      0x62,   // From your old code
        // 0xB8,                                   // SET_GRAYSCALE_LINEAR - Using hex value directly
        // SSD1327_SET_CONTRAST_CURRENT,   0x7F,
        // SSD1327_SET_NORMAL_DISPLAY,             // Corresponds to SET_DISP_MODE
        // 0x2E                                    // SET_SCROLL_DEACTIVATE

        SSD1327_SET_CLOCK_DIV,          0xF1,   // Set Clock Divider / Oscillator Frequency
        SSD1327_SET_MUX_RATIO,          0x7F,   // Set MUX Ratio for 128 rows
        SSD1327_SET_START_LINE,         0x00,   // Set Display Start Line to 0
        SSD1327_SET_DISPLAY_OFFSET,     0x00,   // Set Display Offset to 0
        SSD1327_SET_REMAP,              0x74,   // Set Remap & Color Depth (Adafruit's value)
        SSD1327_SET_FUNC_SELECT_A,      0x01,   // Enable internal VDD regulator
        SSD1327_SET_PHASE_LENGTH,       0x32,   // Set Phase Length
        SSD1327_SET_SECOND_PRECHARGE,   0x0D,   // Set Second Pre-charge Period
        SSD1327_SET_PRECHARGE,          0x0D,   // Set Pre-charge Voltage
        SSD1327_SET_VCOMH,              0x0B,   // Set VCOMH Level
        SSD1327_SET_MASTER_CONTRAST,    0x99,   // Set Master Contrast
        SSD1327_SET_MASTER_CURRENT,     0x0F,   // Set Master Current Control
        
        // Set the Linear Grayscale Table (1 command byte + 15 data bytes)
        SSD1327_SET_GRAYSCALE_TABLE,
        0x0C, 0x18, 0x24, 0x30, 0x3C, 0x48, 0x54, 0x60,
        0x6C, 0x78, 0x84, 0x90, 0x9C, 0xA8, 0xB4,

        // Set Display Mode to Normal
        SSD1327_SET_NORMAL_DISPLAY
    }; 
    
    Display_WriteCommandList(initCommands, sizeof(initCommands));
    sleep_ms(100);
    Display_WriteCommand(SSD1327_DISPLAY_ON);
}

u32 Display_WriteCommand(u8 command) {
    HTRACE("ssd1327.c -> Display_WriteCommand(I2C_Config_t*, u8):b8");

    if(sgI2C == NULL) {
        HWARN("Display_WriteCommand(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[2];
    buffer[0] = 0x00;   // Control Byte: the following data is a command
    buffer[1] = command;

    u32 status = i2c_write_blocking(sgI2C->i2c, SSD1327_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("Display_WriteCommand(): Couldn't write data to device at address: 0x%x", SSD1327_ADDRESS);
        return false;
    } else return status;
}

u32 Display_WriteCommandList(const u8* commands, size_t len) {
    HTRACE("ssd1327.c -> Display_WriteCommandList(I2C_Config_t*, u8*, size_t):b8");

    if(sgI2C == NULL) {
        HWARN("Display_WriteCommandList(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[len+1];
    buffer[0] = 0x00;   // Control Byte: the following data is a command
    memcpy(buffer+1, commands, len);

    u32 status = i2c_write_blocking(sgI2C->i2c, SSD1327_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("Display_WriteCommandList(): Couldn't write data to device at address: 0x%x", SSD1327_ADDRESS);
        return false;
    } else return status;
}

u32 Display_WriteData(const u8* data, size_t len) {
    HTRACE("ssd1327.c -> Display_WriteData(I2C_Config_t*, u8*, size_t):b8");

    if(sgI2C == NULL) {
        HWARN("SH1107_WriteCommandList(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[len+1];
    buffer[0] = 0x40;   // Control Byte: the following data is a pixel
    memcpy(buffer+1, data, len);

    u32 status = i2c_write_blocking(sgI2C->i2c, SSD1327_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("Display_WriteData(): Couldn't write data to device at address: 0x%x", SSD1327_ADDRESS);
        return false;
    } else return status;
}

#endif