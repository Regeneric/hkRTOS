// Newlib
#include <stdio.h>

// Pico SDK
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

// Segger
#include <SEGGER_SYSVIEW.h>

// hkRTOS
#include <defines.h>

#include <comm/i2c.h>
#include <comm/wifi.h>

#include <storage/storage.h>
#include <storage/eeprom.h>

#include <sensors/sensors.h>
#include <sensors/dht11.h>


void main(void) {
    stdio_init_all();

    I2C_Config_t hkI2C0 = {
        .i2c   = hkI2C,
        .sda   = hkI2C_SDA,
        .scl   = hkI2C_SCL,
        .speed = hkI2C_SPEED,
    }; I2C_Init(&hkI2C0);

    WIFI_Config_t hkWIFI = {
        .ssid      = hkWIFI_SSID,
        .password  = hkWIFI_PASS,
        .authType  = CYW43_AUTH_WPA2_AES_PSK,
        .country   = CYW43_COUNTRY_POLAND
    }; WIFI_Init(&hkWIFI);

    vTaskStartScheduler();
}























// void main(void) {
//     stdio_init_all();

//     I2C_Config_t hkI2C0 = {
//         .i2c   = hkI2C,
//         .sda   = hkI2C_SDA,
//         .scl   = hkI2C_SCL,
//         .speed = hkI2C_SPEED,
//     }; I2C_Init(&hkI2C0);
 
//     u8 dataBytes[5];
//     DHT_Config_t hkDHT11 = {
//         .gpio  = hkDHT11_PIN,
//         .data  = dataBytes,
//         .queue = NULL
//     }; DHT11_Init(&hkDHT11);

//     DataPacket_t eepromWrite = {
//         .address = 0x00,
//         .data    = dataBytes,
//         .size    = sizeof(dataBytes)
//     };

//     u8 eepromBytes[5];
//     DataPacket_t eepromRead = {
//         .address = 0x00,
//         .data    = eepromBytes,
//         .size    = sizeof(eepromBytes)
//     };

//     f32 humidity    = 0.0f;
//     f32 temperature = 0.0f;

//     WIFI_Config_t hkWIFI = {
//         .ssid      = hkWIFI_SSID,
//         .password  = hkWIFI_PASS,
//         .authType  = CYW43_AUTH_WPA2_AES_PSK,
//         .country   = CYW43_COUNTRY_POLAND
//     }; // WIFI_Init(&hkWIFI);

//     while(FOREVER) {
//         // printf("IP address: %s\n"  , ip4addr_ntoa(hkWIFI.ipAddress));
//         // printf("Network mask: %s\n", ip4addr_ntoa(hkWIFI.ipMask));
//         // printf("Gateway: %s\n\n"   , ip4addr_ntoa(hkWIFI.ipGateway));

//         DHT11_Read(&hkDHT11);

//         humidity    = hkDHT11.data[0] + hkDHT11.data[1] * 0.1f;
//         temperature = hkDHT11.data[2] + hkDHT11.data[3] * 0.1f;
//         if(hkDHT11.data[2] & 0x80) temperature = -temperature;

//         printf("DHT: Temperature: %.1f*C\n"  , temperature);
//         printf("DHT: Humidity:    %.0f%%\n\n", humidity);

//         // vEEPROM_Write(&hkI2C0, &eepromWrite);
//         // sleep_ms(100);
//         // EEPROM_Read(&hkI2C0, &eepromRead);

//         // humidity    = eepromRead.data[0] + eepromRead.data[1] * 0.1f;
//         // temperature = eepromRead.data[2] + eepromRead.data[3] * 0.1f;
//         // if(eepromRead.data[2] & 0x80) temperature = -temperature;

//         // printf("EEP: Temperature: %.1f*C\n"  , temperature);
//         // printf("EEP: Humidity:    %.0f%%\n\n", humidity);

//         sleep_ms(1000);
//     }
// }