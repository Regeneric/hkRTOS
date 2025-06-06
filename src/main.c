// Newlib
#include <stdio.h>

// Pico SDK
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include <hardware/pio.h>

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
#include <sensors/dht11/dht11.h>

static DHT_Config_t hkDHT11;

static bool DHT11_Timer_ISR(struct repeating_timer *t) {
    DHT11_Read(&hkDHT11);
    return true;
}

void main(void) {
    stdio_init_all();

    static u8 dataBytes[5];
        hkDHT11.gpio   = hkDHT_PIN;
        hkDHT11.data   = dataBytes;
        hkDHT11.length = sizeof(dataBytes);
        hkDHT11.queue  = NULL;
        hkDHT11.status = DHT_INIT;
        hkDHT11.pio    = hkPIO;
        hkDHT11.sm     = hkPIO_SM;
    DHT11_Init(&hkDHT11);

    struct repeating_timer timer;
    add_repeating_timer_ms(-2000, DHT11_Timer_ISR, NULL, &timer);

    f32 humidity    = 0.0f;
    f32 temperature = 0.0f;

    while(FOREVER) {
        if(hkDHT11.status != DHT_READ_IN_PROGRESS) {
            if(hkDHT11.status == DHT_READ_SUCCESS) {
                humidity    = hkDHT11.data[0] + hkDHT11.data[1] * 0.1f;
                temperature = hkDHT11.data[2] + hkDHT11.data[3] * 0.1f;
                if(hkDHT11.data[2] & 0x80) temperature = -temperature;

                printf("DHT: Temperature: %.1f*C\n"  , temperature);
                printf("DHT: Humidity:    %.0f%%\n\n", humidity);
                sleep_ms(1000);
            } else {
                printf("DHT: Data read failed\n");
                sleep_ms(500);
            }
        }
    }
}


// void main(void) {
//     stdio_init_all();

//     u8 dataBytes[5];
//     DHT_Config_t hkDHT11 = {
//         .gpio   = hkDHT11_PIN,
//         .data   = dataBytes,
//         .length = sizeof(dataBytes),
//         .queue  = NULL,
//         .status = DHT_INIT,
//         .pio    = hkPIO,
//         .sm     = hkPIO_SM
//     }; DHT11_Init(&hkDHT11);

//     BaseType_t resp = xTaskCreate(DHT11_ReadTask, "DHT11 Read", 128, &hkDHT11, 2, NULL);
//     assert(resp == pdPASS);
    
//     // resp = xTaskCreate();

//     vTaskStartScheduler();
//     while(FOREVER);
// }



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
//         .gpio   = hkDHT11_PIN,
//         .data   = dataBytes,
//         .length = sizeof(dataBytes),
//         .queue  = NULL
//         // .pio    = hkPIO,
//         // .sm  = hkPIO_SM
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

//     // WIFI_Config_t hkWIFI = {
//     //     .ssid      = hkWIFI_SSID,
//     //     .password  = hkWIFI_PASS,
//     //     .authType  = CYW43_AUTH_WPA2_AES_PSK,
//     //     .country   = CYW43_COUNTRY_POLAND
//     // }; WIFI_Init(&hkWIFI);

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