#pragma once
#include <defines.h>

#ifdef HPLATFORM_ARM
    #define HK_LOG_LEVEL                LOG_DEBUG                       // TRACE | DEBUG | INFO | WARN | ERROR

    // Communication
    #define hkI2C_USE_SINGLE_I2C        false                           // Use single or both uC I2Cs
    #define hkI2C_BUS_COUNT            (hkI2C_USE_SINGLE_I2C ? 1 : 2)   // Active I2C buses count
    #if hkI2C_USE_SINGLE_I2C
        #define hkI2C                   i2c0                            // i2c0 or i2c1 ; Default: i2c0
        #define hkI2C_SDA               PICO_DEFAULT_I2C_SDA_PIN        //                Default: 4
        #define hkI2C_SCL               PICO_DEFAULT_I2C_SCL_PIN        //                Default: 5
        #define hkI2C_SPEED_KHZ         1000U                           // 100 ; 400 ; 1000 ; 1700 ; 3400 ; 5000  -  Above 400 KHz use at own risk
        #define hkI2C_SPEED            (hkI2C_SPEED_KHZ * 1000U)        // Conversion from KHz to Hz   
    #else
        #define hkI2C_ONE               i2c0                            // i2c0 or i2c1 ; Default: i2c0
        #define hkI2C_SDA_ONE           PICO_DEFAULT_I2C_SDA_PIN        //                Default: 4
        #define hkI2C_SCL_ONE           PICO_DEFAULT_I2C_SCL_PIN        //                Default: 5
        #define hkI2C_SPEED_KHZ_ONE     400U                            // 100 ; 400 ; 1000 ; 1700 ; 3400 ; 5000  -  Above 400 KHz use at own risk
        #define hkI2C_SPEED_ONE        (hkI2C_SPEED_KHZ_ONE * 1000U)    // Conversion from KHz to Hz   
        
        #define hkI2C_TWO               i2c1                            // i2c0 or i2c1 ; Default: i2c1
        #define hkI2C_SDA_TWO           18                              //              ; Default: 2
        #define hkI2C_SCL_TWO           19                              //              ; Default: 3
        #define hkI2C_SPEED_KHZ_TWO     1000U                           // 100 ; 400 ; 1000 ; 1700 ; 3400 ; 5000  -  Above 400 KHz use at own risk
        #define hkI2C_SPEED_TWO        (hkI2C_SPEED_KHZ_TWO * 1000U)    // Conversion from KHz to Hz   
    #endif
    #define hkENABLE_WIFI               true                            // Enable or disable WiFi module
    #if hkENABLE_WIFI
        #ifndef WIFI_SSID
            #define hkWIFI_SSID         "NP2"                           // WiFi network name
        #else
            #define hkWIFI_SSID         WIFI_SSID                       // If passed from CMake
        #endif
        #ifndef WIFI_PASSWORD 
            #define hkWIFI_PASS         "0123456789"                    // WiFi password
        #else
            #define hkWIFI_PASS         WIFI_PASSWORD                   // If passed from CMake
        #endif
    #endif
    #define hkENABLE_ONEWIRE            true                            // Enable 1-Wire communication
    #if hkENABLE_ONEWIRE
        #define hkOW_PIO                pio1                            // Which PIO
        #define hkOW_PIO_SM             0                               // Starting SM on that PIO; we need 3 in total
        #define hkOW_PIN                13                              // Data pin for the 1-Wire interface
        #define hkOW_USE_DMA            false                           // Decide if we want to use DMA for data collection or CPU
        #if hkOW_USE_DMA            
            #define hkOW_DMA_IRQ        DMA_IRQ_1                       // Which DMA IRQ to use                     
        #endif
    #endif
    #define hkENABLE_UART               true                            // Enable UART communication
    #if hkENABLE_UART
        #define hkUART                  uart1                           // uart0 or uart1 ; Default: uart0
        #define hkUART_TX               8                               // TX GPIO PIN
        #define hkUART_RX               9                               // RX GPIO PIN
        #define hkUART_BAUDRATE         9600                            // Bps
        #define hkUART_DMA_IRQ          DMA_IRQ_1                       // Which DMA IRQ to use
    #endif

    // Sensors
    #define hkDHT_USE_SENSOR            true                            // Decide if we want to use DHT11/DHT22 
    #if hkDHT_USE_SENSOR
        #define hkDHT_PIN               15                              // Data pin of the DHT11/DHT22 sensor
        #define hkDHT_USE_PIO           true                            // Decide if we want to use PIO or CPU
        #if hkDHT_USE_PIO
            #define hkDHT_PIO           pio0                            // Which PIO
            #define hkDHT_PIO_SM        0                               // Which SM on that PIO
            #define hkDHT_USE_DMA       true                            // Decide if we want to use DMA for data collection or CPU
            #if hkDHT_USE_DMA
                #define hkDHT_DMA_IRQ   DMA_IRQ_0                       // Which DMA IRQ to use   
            #endif
        #endif
    #endif
    #define hkBME280_USE_SENSOR         true                            // Decide if we want to use BME280
    #if hkBME280_USE_SENSOR
        #define hkBME280_USE_I2C        true                            // true - i2c ; false - spi
    #endif
    #define hkSGP30_USE_SENSOR          true                            // Decide if we want to use SGP30
    #if hkSGP30_USE_SENSOR
        #define hkSGP30_ADDRESS         0x58                            // i2c address
    #endif
    #define hkPMS5003_USE_SENSOR        true                            // Decide if we want to use PMS5003
    #if hkPMS5003_USE_SENSOR
        #define hkPMS5003aaa
    #endif


    // Storage
    #define hkEEPROM_24LC01B            true                            // 1Kb   (128B) I2C EEPROM
    #if hkEEPROM_24LC01B
        #define hkEEPROM_24AA01         true                            // 1Kb   (128B) I2C EEPROM - compatible with 24LC01B - operating voltage from 1.7V
        #define hkEEPROM_24FC01         false                           // 1Kb   (128B) I2C EEPROM - compatible with 24LC01B - clock up to 1000 KHz
    #endif
    #define hkEEPROM_CAT24C512          false                           // 512Kb (64KB) I2C EEPROM
    #define hkFRAM_MB85RC256V           false                           // 256Kb (32KB) I2C FRAM
    #define hkFLASH_W25Q128JV           true                            // 128Mb (16MB) SPI FLASH

    // Display
    #define hkLCD_PCD8544               false                           // Nokia 5110 LCD Display
    #define hkOLED_SSD1327              true                            // Waveshare 13992 1.5" OLED
    #if hkOLED_SSD1327
        #define hkSSD1327_USE_I2C       true                            // true - I2C ; false - 4 Wire SPI
    #endif
    #define hkOLED_SH1107               false
    #if hkOLED_SH1107
        #define hkSH1107_USE_I2C        true                            // true - I2C ; false - 4 Wire SPI
    #endif

    // Input
    #define hkKY40_ENCODER              true                            // Keyes KY-040 Rotary Encoder
    #if hkKY40_ENCODER
        #define hkKY40_CLK_PIN          16                              // CLK GPIO PIN
        #define hkKY40_DT_PIN           17                              // DT GPIO PIN
        #define hkKY40_BTN_PIN          20                              // Button GPIO PIN
        #define hkKY40_ENCODERS_COUNT   1                               // How many encoders will be used in a project
        #define hkKY40_GET              -1                              // Helper define for KY40_Position() function
    #endif

    // Debugging
    #define hkSEGGER_SYSVIEW            true                            // Use SEGGER SysView for debugging
    #if hkSEGGER_SYSVIEW
        #define HPRINT(val)             SEGGER_SYSVIEW_PrintfHost(val)  // Shorthand macro
    #else
        #define HPRINT(val)
    #endif
#endif