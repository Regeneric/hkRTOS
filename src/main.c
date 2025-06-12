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

#include <core/logger.h>

#include <comm/i2c.h>
#include <comm/uart.h>
#include <comm/network/wifi.h>
#include <comm/onewire/onewire.h>

#include <storage/storage.h>
#include <storage/eeprom.h>

#include <sensors/sensors.h>
#include <sensors/dht11/dht11.h>
#include <sensors/ds18b20/ds18b20.h>
#include <sensors/pms5003/pms5003.h>

#include <display/display.h>
#include <display/gfx/gfx.h>
#include <display/ssd1327/ssd1327_config.h>

static DHT_Config_t hkDHT11;
static UART_Config_t uart;
static PMS5003_Config_t hkPMS5003;

static bool DHT11_Timer_ISR(struct repeating_timer* t) {
    DHT11_Read(&hkDHT11);
    return true;
}

static bool PMS5003_Timer_ISR(struct repeating_timer* t) {
    PMS5003_Read(&uart, &hkPMS5003);
    return true;
}


void main(void) {
    stdio_init_all();

    I2C_Config_t i2c = {
        .i2c   = hkI2C,
        .scl   = hkI2C_SCL,
        .sda   = hkI2C_SDA,
        .speed = hkI2C_SPEED
    }; I2C_Init(&i2c);

    DisplayConfig_t hkSSD1327 = {
        .width    = 128,
        .height   = 128,
        .address  = SSD1327_ADDRESS
    }; // Display_Init(&i2c, &hkSSD1327);


    // static UART_Config_t uart;
    static u8 hkUART_RawData[32];
    uart.uart     = hkUART;
    uart.tx       = hkUART_TX;
    uart.rx       = hkUART_RX;
    uart.baudrate = hkUART_BAUDRATE;
    uart.data     = hkUART_RawData;
    uart.length   = sizeof(hkUART_RawData);
    uart.status   = UART_INIT;
    uart.packetSize = 32;   // How many bytes will be read from a single transmission
    UART_Init(&uart);

    // static PMS5003_Config_t hkPMS5003;
    static u8 hkPMS5003_RawData[32];
    hkPMS5003.pm1    = 0;
    hkPMS5003.pm2_5  = 0;
    hkPMS5003.pm10   = 0;
    hkPMS5003.rawData = hkPMS5003_RawData;
    hkPMS5003.length  = sizeof(hkPMS5003_RawData);

    PMS5003_DataPacket_t hkPMS5003_Data = {
        .pm1   = 0,
        .pm2_5 = 0,
        .pm10  = 0,
        .jsonify = PMS5003_Jsonify
    };


    OneWire_Config_t ow0 = {
        .gpio    = hkOW_PIN,
        .pio     = hkOW_PIO,
        .sm      = hkOW_PIO_SM,
        .status  = ONEW_INIT,
        .bitMode = 8
    }; OneWire_Init(&ow0);

    static u8 hkDS18B20_RawData[9];
    DS18B20_Config_t hkDS18B20 = {
        // .address     = 0x2836C7BE000000B6
        .address     = ONEW_SKIP_ROM,
        .data        = hkDS18B20_RawData,
        .length      = sizeof(hkDS18B20_RawData),
        .queue       = NULL,
        .temperature = 0.0f
    }; 

    DS18B20_DataPacket_t hkDS18B20_Data = {
        .temperature = 0.0f,
        .address = 0,
        .jsonify = DS18B20_Jsonify
    };


    // static DHT_Config_t hkDHT11;
    static u8 hkDHT_RawData[5];
    hkDHT11.gpio   = hkDHT_PIN;
    hkDHT11.pio    = hkDHT_PIO;
    hkDHT11.sm     = hkDHT_PIO_SM;
    hkDHT11.queue  = NULL;
    hkDHT11.status = DHT_INIT;
    hkDHT11.data   = hkDHT_RawData;
    hkDHT11.length = sizeof(hkDHT_RawData);
    DHT11_Init(&hkDHT11);

    DHT_DataPacket_t hkDHT11_Data = {
        .humidity    = 0.0f,
        .temperature = 0.0f,
        .jsonify = DHT11_Jsonify
    };


    struct repeating_timer hkDHT11_Timer;
    struct repeating_timer hkPMS5003_Timer;

    add_repeating_timer_ms(-2000, DHT11_Timer_ISR, NULL, &hkDHT11_Timer);
    add_repeating_timer_ms(-2300, PMS5003_Timer_ISR, NULL, &hkPMS5003_Timer);

    
    while(FOREVER) {
        DS18B20_Read(&ow0, &hkDS18B20);
        HDEBUG("[DS18B20] TEMP: %.2f*C", hkDS18B20.temperature);
        char* hkDS18B20_Json = hkDS18B20_Data.jsonify(&hkDS18B20_Data);

        if(hkDHT11.status == DHT_READ_SUCCESS) {
            DHT11_ProcessData(&hkDHT11, &hkDHT11_Data);
            HDEBUG("[DHT11] TEMP: %.2f*C", hkDHT11_Data.temperature);
            HDEBUG("[DHT11] HUMI: %.2f%%", hkDHT11_Data.humidity);
            char* hkDHT11_Json = hkDHT11_Data.jsonify(&hkDHT11_Data);
        }

        if(uart.status == UART_DATA_RX_SUCCESS) {
            PMS5003_ProcessData(&uart, &hkPMS5003);
            HDEBUG("[PM 1  ]: %u ug/m3"  , hkPMS5003.pm1);
            HDEBUG("[PM 2.5]: %u ug/m3"  , hkPMS5003.pm2_5);
            HDEBUG("[PM 10 ]: %u ug/m3\n", hkPMS5003.pm10);
            char* hkPMS5003_Json = hkPMS5003_Data.jsonify(&hkPMS5003_Data);
        }

        sleep_ms(1000);
    }
}