#include <config/arm.h>
#if hkOLED_SSD1327

#include <string.h>

#include <hardware/i2c.h>

#include <core/gpio.h>

#include <display/display.h>
#include <display/ssd1327/ssd1327_config.h>

static I2C_Config_t* sgI2C;

static inline void Display_Reset() {
    HTRACE("ssd1327.c -> s:Display_Reset(void):void");

    gpio_init(SSD1327_RESET_PIN);
    gpio_set_dir(SSD1327_RESET_PIN, GPIO_OUT);
    
    gpio_put(SSD1327_RESET_PIN, GPIO_HIGH);
    sleep_ms(1);
    
    gpio_put(SSD1327_RESET_PIN, GPIO_LOW);
    sleep_ms(1);
    
    gpio_put(SSD1327_RESET_PIN, GPIO_HIGH);
    sleep_ms(1);
}

b8 Display_Init(I2C_Config_t* i2c, DisplayConfig_t* config) {
    HTRACE("ssd1327.c -> Display_Init(I2C_Config_t*, DisplayConfig_t*):b8");
    
    Display_Reset();

    sgI2C = i2c;
    static const u8 initCommands[] = {
        SSD1327_SET_COMMAND_LOCK,       0x12,
        SSD1327_DISPLAY_OFF,         // 0xAE
        SSD1327_SET_CLOCK_DIV,          0xF1,   // Set Clock Divider / Oscillator Frequency
        SSD1327_SET_MUX_RATIO,          0x7F,   // Set MUX Ratio for 128 rows           0x3F for 64
        SSD1327_SET_DISPLAY_OFFSET,     0x00,   // Set Display Offset to 0
        SSD1327_SET_START_LINE,         0x00,   // Set Display Start Line to 0
        SSD1327_SET_REMAP,              0x71,   // Set Remap & Color Depth
        SSD1327_SET_FUNC_SELECT_A,      0x01,   // Enable internal VDD regulator
        SSD1327_SET_CONTRAST_CURRENT,   0x9F,   // Set contrast
        SSD1327_SET_MASTER_CURRENT,     0x0F,   // Set Master Current Control
        SSD1327_SET_PRECHARGE,          0x0D,   // Set Pre-charge Voltage
        SSD1327_SET_PHASE_LENGTH,       0x32,   // Set Phase Length
        SSD1327_SET_VCOMH,              0x0B,   // Set VCOMH Level
        SSD1327_SET_SECOND_PRECHARGE,   0x0D,   // Set Second Pre-charge Period
        SSD1327_SET_MASTER_CONTRAST,    0x99,   // Set Master Contrast
        
        // Set the Linear Grayscale Table (1 command byte + 15 data bytes)
        // SSD1327_SET_GRAYSCALE_TABLE,
        // 0x0C, 0x18, 0x24, 0x30, 0x3C, 0x48, 0x54, 0x60,
        // 0x6C, 0x78, 0x84, 0x90, 0x9C, 0xA8, 0xB4,

        // // Set Display Mode to Normal
        // SSD1327_SET_NORMAL_DISPLAY   // 0xA6
    }; 
    
    Display_WriteCommandList(initCommands, sizeof(initCommands));
    sleep_ms(100);
    Display_WriteCommand(SSD1327_DISPLAY_ON);   // 0xAF
}

u32 Display_WriteCommand(u8 command) {
    HTRACE("ssd1327.c -> Display_WriteCommand(u8):b8");

    if(sgI2C == NULL) {
        HWARN("Display_WriteCommand(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[2];
    buffer[0] = 0x00;   // Control Byte: the following data is a command
    buffer[1] = command;

    mutex_enter_blocking(sgI2C->mutex);
    HTRACE("Display_WriteCommand(): Mutex acquired");

    u32 status = i2c_write_blocking(sgI2C->i2c, SSD1327_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("Display_WriteCommand(): Couldn't write data to device at address: 0x%x", SSD1327_ADDRESS);
        mutex_exit(sgI2C->mutex);
        HTRACE("Display_WriteCommand(): Mutex released");
        return false;
    } 
    
    mutex_exit(sgI2C->mutex);
    HTRACE("Display_WriteCommand(): Mutex released");
    return status;
}

u32 Display_WriteCommandList(const u8* commands, size_t len) {
    HTRACE("ssd1327.c -> Display_WriteCommandList(u8*, size_t):u32");

    if(sgI2C == NULL) {
        HWARN("Display_WriteCommandList(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[len+1];
    buffer[0] = 0x00;   // Control Byte: the following data is a command
    memcpy(buffer+1, commands, len);

    mutex_enter_blocking(sgI2C->mutex);
    HTRACE("Display_WriteCommandList(): Mutex acquired");
    
    u32 status = i2c_write_blocking(sgI2C->i2c, SSD1327_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("Display_WriteCommandList(): Couldn't write data to device at address: 0x%x", SSD1327_ADDRESS);
        mutex_exit(sgI2C->mutex);
        HTRACE("Display_WriteCommandList(): Mutex released");
        return false;
    } 
    
    mutex_exit(sgI2C->mutex);
    HTRACE("Display_WriteCommandList(): Mutex released");
    return status;
}

u32 Display_WriteData(const u8* data, size_t len) {
    HTRACE("ssd1327.c -> Display_WriteData(u8*, size_t):b8");

    if(sgI2C == NULL) {
        HWARN("Display_WriteData(): I2C instance is not initialized!");
        return false;
    }

    u8 buffer[len+1];
    buffer[0] = 0x40;   // Control Byte: the following data is a pixel
    memcpy(buffer+1, data, len);

    mutex_enter_blocking(sgI2C->mutex);
    HTRACE("Display_WriteData(): Mutex acquired");

    u32 status = i2c_write_blocking(sgI2C->i2c, SSD1327_ADDRESS, buffer, sizeof(buffer), false);
    if(status == PICO_ERROR_GENERIC) {
        HWARN("Display_WriteData(): Couldn't write data to device at address: 0x%x", SSD1327_ADDRESS);
        mutex_exit(sgI2C->mutex);
        HTRACE("Display_WriteData(): Mutex released");
        return false;
    } 
    
    mutex_exit(sgI2C->mutex);
    HTRACE("Display_WriteData(): Mutex released");
    return status;
}

#endif