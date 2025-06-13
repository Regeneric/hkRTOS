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

#include <core/map.h>
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
        .address  = SSD1327_ADDRESS,
        .textSize = 1,
        .status   = DISP_DRAW
    }; Display_Init(&i2c, &hkSSD1327);


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
    struct repeating_timer hkSSD1327_Timer;

    add_repeating_timer_ms(-2000, DHT11_Timer_ISR, NULL, &hkDHT11_Timer);
    add_repeating_timer_ms(-2300, PMS5003_Timer_ISR, NULL, &hkPMS5003_Timer);
    // add_repeating_timer_ms(-10000, SSD1327_Timer_ISR, NULL, &hkSSD1327_Timer);

    // hkDrawTestPattern();    // Display test pattern
    char buffer[64];
    
    hkClearBuffer();

    static u8 DS18B20_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DS18B20_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,         // Start Y point
        .height = 50,   // End Y point  
        .minVal = 10.0,
        .maxVal = 50.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = DS18B20_GraphHistory,
        .length  = sizeof(DS18B20_GraphHistory)
    }; hkGraphInit(&DS18B20_Graph);

    static u8 PMS5003_2_5_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t PMS5003_PM2_5_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 73,        // Start Y point
        .height = 50,   // End Y point   
        .minVal = 0.0,
        .maxVal = 150.0,
        .colour = 10,
        .borderColour = 10,
        .legendColour = 4,
        .cursorX = 0,
        .history = PMS5003_2_5_GraphHistory,
        .length  = sizeof(PMS5003_2_5_GraphHistory)
    }; hkGraphInit(&PMS5003_PM2_5_Graph);

    hkClearBuffer();
    hkGraphDrawAxes(&DS18B20_Graph);
    hkGraphDrawAxes(&PMS5003_PM2_5_Graph);
    hkDisplay();

    while(FOREVER) {
        DS18B20_ReadAndProcess(&ow0, &hkDS18B20, &hkDS18B20_Data);
        HDEBUG("[DS18B20] TEMP: %.2f*C", hkDS18B20_Data.temperature);
        char* hkDS18B20_Json = hkDS18B20_Data.jsonify(&hkDS18B20_Data);

        if(hkDHT11.status == DHT_READ_SUCCESS) {
            DHT11_ProcessData(&hkDHT11, &hkDHT11_Data);
            HDEBUG("[DHT11] TEMP: %.2f*C", hkDHT11_Data.temperature);
            HDEBUG("[DHT11] HUMI: %.2f%%", hkDHT11_Data.humidity);
            char* hkDHT11_Json = hkDHT11_Data.jsonify(&hkDHT11_Data);
        }

        if(uart.status == UART_DATA_RX_SUCCESS) {
            PMS5003_ProcessData(&hkPMS5003, &hkPMS5003_Data);
            HDEBUG("[PMS5003] PM   1: %u ug/m3"  , hkPMS5003_Data.pm1);
            HDEBUG("[PMS5003] PM 2.5: %u ug/m3"  , hkPMS5003_Data.pm2_5);
            HDEBUG("[PMS5003] PM  10: %u ug/m3\n", hkPMS5003_Data.pm10);
            char* hkPMS5003_Json = hkPMS5003_Data.jsonify(&hkPMS5003_Data);
        }

        if(hkSSD1327.status == DISP_DRAW) {
            // hkDrawGraph(hkDS18B20_Data.temperature);
            hkGraphAddDataPoint(&DS18B20_Graph, hkDS18B20_Data.temperature);
            hkGraphAddDataPoint(&PMS5003_PM2_5_Graph, (f32)hkPMS5003_Data.pm2_5);
            
            hkClearBuffer();
            hkGraphDrawAxes(&DS18B20_Graph);
            hkGraphDrawLegend(&DS18B20_Graph, "TEMP (C)");

            hkGraphDrawAxes(&PMS5003_PM2_5_Graph);
            hkGraphDrawLegend(&PMS5003_PM2_5_Graph, "PM2.5");

            hkGraphDraw(&DS18B20_Graph);
            hkGraphDraw(&PMS5003_PM2_5_Graph);

            hkDisplay();

            // hkClearBuffer();

            // snprintf(buffer, sizeof(buffer), "IT: %.2f *C", hkDHT11_Data.temperature);
            // hkDrawFastString(2, 2, buffer);

            // snprintf(buffer, sizeof(buffer), "OT: %.2f *C", hkDS18B20_Data.temperature);
            // hkDrawFastString(2, 12, buffer);

            // snprintf(buffer, sizeof(buffer), "H : %.0f %%", hkDHT11_Data.humidity);
            // hkDrawFastString(2, 22, buffer);

            // snprintf(buffer, sizeof(buffer), "PM 1  : %u ug/m3", hkPMS5003_Data.pm1);
            // hkDrawFastString(2, 42, buffer);

            // snprintf(buffer, sizeof(buffer), "PM 2.5: %u ug/m3", hkPMS5003_Data.pm2_5);
            // hkDrawFastString(2, 52, buffer);

            // snprintf(buffer, sizeof(buffer), "PM 10 : %u ug/m3", hkPMS5003_Data.pm10);
            // hkDrawFastString(2, 62, buffer);
            
            // hkDisplay();
        }

        sleep_ms(1000);
    }
}