#pragma once

#ifdef HPLATFORM_ARM
    // Communication
    #define hkI2C_USE_SINGLE_I2C        true                            // Use single or both uC I2Cs
    #define hkI2C_BUS_COUNT            (hkI2C_USE_SINGLE_I2C ? 1 : 2)   // Active I2C buses count
    #if hkI2C_USE_SINGLE_I2C
        #define hkI2C                   i2c0                            // i2c0 or i2c1 ; Default: i2c0
        #define hkI2C_SDA               PICO_DEFAULT_I2C_SDA_PIN        //                Default: 4
        #define hkI2C_SCL               PICO_DEFAULT_I2C_SCL_PIN        //                Default: 5
        #define hkI2C_SPEED_KHZ         400U                            // 100 ; 400 ; 1000 ; 1700 ; 3400 ; 5000  -  Above 400 KHz use at own risk
        #define hkI2C_SPEED            (hkI2C_SPEED_KHZ * 1000U)        // Conversion from KHz to Hz   
    #else
        #define hkI2C_ONE               i2c0                            // i2c0 or i2c1 ; Default: i2c0
        #define hkI2C_SDA_ONE           PICO_DEFAULT_I2C_SDA_PIN        //                Default: 4
        #define hkI2C_SCL_ONE           PICO_DEFAULT_I2C_SCL_PIN        //                Default: 5
        #define hkI2C_SPEED_KHZ_ONE     400U                            // 100 ; 400 ; 1000 ; 1700 ; 3400 ; 5000  -  Above 400 KHz use at own risk
        #define hkI2C_SPEED_ONE        (hkI2C_SPEED_KHZ_ONE * 1000U)    // Conversion from KHz to Hz   
        
        #define hkI2C_TWO               i2c1                            // i2c0 or i2c1 ; Default: i2c1
        #define hkI2C_SDA_TWO           2                               //              ; Default: 2
        #define hkI2C_SCL_TWO           3                               //              ; Default: 3
        #define hkI2C_SPEED_KHZ_TWO     400U                            // 100 ; 400 ; 1000 ; 1700 ; 3400 ; 5000  -  Above 400 KHz use at own risk
        #define hkI2C_SPEED_TWO        (hkI2C_SPEED_KHZ_TWO * 1000U)    // Conversion from KHz to Hz   
    #endif
    #define hkSEGGER_SYSVIEW            true                            // Use SEGGER SysView for debugging
    #if hkSEGGER_SYSVIEW
        #define HPRINT(val)             SEGGER_SYSVIEW_PrintfHost(val)  // Shorthand macro
    #else
        #define HPRINT(val)
    #endif

    // Sensors
    #define hkDHT11_PIN                 15                              // Data pin of the DHT11 sensor
    #define hkDHT11_USE_PIO             true                            // Decide if we want to use PIO or CPU
    #if hkDHT11_USE_PIO
        #define hkPIO                   pio0                            // Which PIO
        #define hkPIO_SM                0                               // Which SM on that PIO
    #else
        #define hkPIO
        #define hkPIO_SM
    #endif

    // Storage
    #define hkEEPROM_24LC01B            true                            // 1Kb   (128B) I2C EEPROM
    // #define hkEEPROM_CAT24C512       true                            // 512Kb (64KB) I2C EEPROM
    // #define hkFRAM_MB85RC256V        true                            // 256Kb (32KB) I2C FRAM

    // Communication
    #define hkENABLE_WIFI               false
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
    #else
        #define hkWIFI_SSID
        #define hkWIFI_PASS
    #endif

    // Display
    #define hkLCD_PCD8544               true                            // Nokia 5110 LCD Display
    // #define hkOLED_SSD1306           true                            // 1.5" OLED   

#endif