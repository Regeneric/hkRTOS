// Newlib
#include <stdio.h>

// Pico SDK
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>
#include <pico/cyw43_arch.h>
#include <pico/sync.h>
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

#include <comm/dma_irq_handler.h>
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
#include <sensors/sgp30/sgp30.h>
#include <sensors/bme280/bme280.h>

#include <display/display.h>
#include <display/gfx/gfx.h>
#include <display/ssd1327/ssd1327_config.h>

static DHT_Config_t hkDHT11;
static UART_Config_t uart;
static I2C_Config_t  hkI2C0;
static I2C_Config_t  hkI2C1;
static PMS5003_Config_t hkPMS5003;
static DS18B20_Config_t hkDS18B20;
static SGP30_Config_t   hkSGP30;
static BME280_Config_t  hkBME280;

static mutex_t hkI2C0_Mutex;
static mutex_t hkI2C1_Mutex;


queue_t displayDataQueue;
extern void hkDisplayLoop();

static bool DHT11_Timer_ISR(struct repeating_timer* t) {
    DHT11_Read(&hkDHT11);
    return true;
}

static bool PMS5003_Timer_ISR(struct repeating_timer* t) {
    PMS5003_Read(&uart, &hkPMS5003);
    return true;
}

static bool DS18B20_Timer_ISR(struct repeating_timer* t) {
    DS18B20_Read(&hkDS18B20);
    return true;
}

static i64 SGP30_Read_Callback(alarm_id_t id, void* data) {
    SGP30_Read(&hkI2C0, &hkSGP30);
    return false;
}

static bool SGP30_Timer_ISR(struct repeating_timer* t) {
    SGP30_InitRead(&hkI2C0, &hkSGP30);
    add_alarm_in_ms(15, SGP30_Read_Callback, NULL, true);
    return true;
}

static i64 SGP30_Baseline_Callback(alarm_id_t id, void* data) {
    SGP30_GetBaseline(&hkI2C0, &hkSGP30);
    return false;
}

static bool SGP30_Baseline_Timer_ISR(struct repeating_timer* t) {
    SGP30_InitGetBaseline(&hkI2C0, &hkSGP30);
    add_alarm_in_ms(15, SGP30_Baseline_Callback, NULL, true);
    return true;
}

static i64 BME280_Read_Callback(alarm_id_t id, void* data) {
    BME280_Read(&hkI2C0, &hkBME280);
    return false;
}

static bool BME280_Timer_ISR(struct repeating_timer* t) {
    BME280_InitRead(&hkI2C0, &hkBME280);
    add_alarm_in_ms(15, BME280_Read_Callback, NULL, true);
    return true;
}


void main(void) {
    stdio_init_all();
    DMA_Master_Init();

    mutex_init(&hkI2C0_Mutex);
    hkI2C0.i2c   = hkI2C_ONE;
    hkI2C0.scl   = hkI2C_SCL_ONE;
    hkI2C0.sda   = hkI2C_SDA_ONE;
    hkI2C0.speed = hkI2C_SPEED_ONE;
    hkI2C0.mutex = &hkI2C0_Mutex;
    I2C_Init(&hkI2C0);

    mutex_init(&hkI2C1_Mutex);
    hkI2C1.i2c   = hkI2C_TWO;
    hkI2C1.scl   = hkI2C_SCL_TWO;
    hkI2C1.sda   = hkI2C_SDA_TWO;
    hkI2C1.speed = hkI2C_SPEED_TWO;
    hkI2C1.mutex = &hkI2C1_Mutex;
    I2C_Init(&hkI2C1);

    DisplayConfig_t hkSSD1327 = {
        .width    = 128,
        .height   = 128,
        .address  = SSD1327_ADDRESS,
        .textSize = 1,
        .status   = DISP_DRAW
    }; Display_Init(&hkI2C1, &hkSSD1327);


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
    // .address     = 0x2836C7BE000000B6
    hkDS18B20.address     = ONEW_SKIP_ROM;
    hkDS18B20.data        = hkDS18B20_RawData;
    hkDS18B20.length      = sizeof(hkDS18B20_RawData);
    hkDS18B20.queue       = NULL;
    hkDS18B20.temperature = 0.0f;
    hkDS18B20.dataCount   = 0;
    hkDS18B20.state       = DS18B20_STATE_IDLE;
    hkDS18B20.convertStartTime = 0;
    DS18B20_SetResolution(&ow0, 11);

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

    static u8 hkSGP30_RawData[6];
    hkSGP30.rawData = hkSGP30_RawData;
    hkSGP30.length  = sizeof(hkSGP30_RawData);
    hkSGP30.status  = hkSGP_INIT;
    hkSGP30.eco2Baseline = 0;
    hkSGP30.tvocBaseline = 0;
    SGP30_Init(&hkI2C0, &hkSGP30);

    SGP30_DataPacket_t hkSGP30_Data = {
        .eco2 = 0,
        .tvoc = 0,
        .jsonify = SGP30_Jsonify
    };


    static u8 hkBME280_RawData[8];
    hkBME280.rawData = hkBME280_RawData;
    hkBME280.length  = sizeof(hkBME280_RawData);
    hkBME280.status  = BME_INIT;
    BME280_Init(&hkI2C0, &hkBME280);

    BME280_DataPacket_t hkBME280_Data = {
        .humidity = 0.0f,
        .pressure = 0.0f,
        .temperature = 0.0f,
        .jsonify = NULL
    };

    struct repeating_timer hkDHT11_Timer;
    struct repeating_timer hkPMS5003_Timer;
    struct repeating_timer hkDS18B20_Timer;
    struct repeating_timer hkSGP30_Timer;
    struct repeating_timer hkSGP30_Baseline_Timer;
    struct repeating_timer hkBME280_Timer;

    add_repeating_timer_ms(-2000, DHT11_Timer_ISR, NULL, &hkDHT11_Timer);
    add_repeating_timer_ms(-2300, PMS5003_Timer_ISR, NULL, &hkPMS5003_Timer);
    // add_repeating_timer_ms(-100, DS18B20_Timer_ISR, NULL, &hkDS18B20_Timer);
    add_repeating_timer_ms(-1000, SGP30_Timer_ISR, NULL, &hkSGP30_Timer);
    // add_repeating_timer_ms(-1500, SGP30_Baseline_Timer_ISR, NULL, &hkSGP30_Baseline_Timer);
    add_repeating_timer_ms(-1200, BME280_Timer_ISR, NULL, &hkBME280_Timer);


    queue_init(&displayDataQueue, sizeof(Sensors_DataPacket_t), 2);
    multicore_launch_core1(hkDisplayLoop);
    HINFO("[CORE1]: Core started for display rendering");

    while(FOREVER) {
        DS18B20_ReadAndProcess(&ow0, &hkDS18B20, &hkDS18B20_Data);
        HDEBUG("[DS18B20] TEMP: %.2f*C", hkDS18B20_Data.temperature);
        char* hkDS18B20_Json = hkDS18B20_Data.jsonify(&hkDS18B20_Data);

        // DS18B20_Read(&hkDS18B20);
        // DS18B20_Tick(&ow0, &hkDS18B20, &hkDS18B20_Data);
        // if(hkDS18B20_Data.status == DS18B20_DATA_READY) {
        //     hkDS18B20_Data.status = DS18B20_DATA_PROCESSED;
        //     HDEBUG("[DS18B20] TEMP: %.2f*C", hkDS18B20_Data.temperature);
        //     char* hkDS18B20_Json = hkDS18B20_Data.jsonify(&hkDS18B20_Data);
        // }

        if(hkDHT11.status == DHT_READ_SUCCESS) {
            DHT11_ProcessData(&hkDHT11, &hkDHT11_Data);
            HDEBUG("[ DHT11 ] TEMP: %.2f*C", hkDHT11_Data.temperature);
            HDEBUG("[ DHT11 ] HUMI: %.2f%%", hkDHT11_Data.humidity);
            char* hkDHT11_Json = hkDHT11_Data.jsonify(&hkDHT11_Data);
        }

        if(uart.status == UART_DATA_RX_SUCCESS) {
            PMS5003_ProcessData(&hkPMS5003, &hkPMS5003_Data);
            HDEBUG("[PMS5003] PM   1: %u ug/m3"  , hkPMS5003_Data.pm1);
            HDEBUG("[PMS5003] PM 2.5: %u ug/m3"  , hkPMS5003_Data.pm2_5);
            HDEBUG("[PMS5003] PM  10: %u ug/m3", hkPMS5003_Data.pm10);
            char* hkPMS5003_Json = hkPMS5003_Data.jsonify(&hkPMS5003_Data);
        }

        if(hkSGP30.status == hkSGP_READY) {
            SGP30_ProcessData(&hkSGP30, &hkSGP30_Data);
            HDEBUG("[ SGP30 ] eCO2: %u ppm", hkSGP30_Data.eco2);
            HDEBUG("[ SGP30 ] TVOC: %u ppb", hkSGP30_Data.tvoc);
            char* hkSGP30_Json = hkSGP30_Data.jsonify(&hkSGP30_Data);
        }

        if(hkSGP30.status == hkSGP_BASELINE_READY) {
            SGP30_ProcessBaseline(&hkSGP30);
            HDEBUG("[ SGP30 ] BeCO2: %u", hkSGP30.eco2Baseline);
            HDEBUG("[ SGP30 ] BTVOC: %u", hkSGP30.tvocBaseline);
        }

        if(hkBME280.status == BME_READ_SUCCESS) {
            BME280_ProcessData(&hkBME280, &hkBME280_Data);
            HDEBUG("[BME280 ] TEMP: %.2f*C", hkBME280_Data.temperature);
            HDEBUG("[BME280 ] HUMI: %.2f%%", hkBME280_Data.humidity);
            HDEBUG("[BME280 ] PRESS: %.1f hPa", hkBME280_Data.pressure);
        }

        if(hkSSD1327.status == DISP_DRAW) {
            Sensors_DataPacket_t dataToDisplay;
            
            dataToDisplay.dht = hkDHT11_Data;
            dataToDisplay.ds18b20 = hkDS18B20_Data;
            dataToDisplay.pms5003 = hkPMS5003_Data;
            dataToDisplay.sgp30   = hkSGP30_Data;
            dataToDisplay.bme280  = hkBME280_Data;

            queue_try_add(&displayDataQueue, &dataToDisplay);
        }

        printf("\n");
        // sleep_ms(1000);
    }
}