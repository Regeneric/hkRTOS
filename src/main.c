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
#include <sensors/dht11_22/dht11_22.h>
#include <sensors/ds18b20/ds18b20.h>
#include <sensors/pms5003/pms5003.h>
#include <sensors/sgp30/sgp30.h>
#include <sensors/bme280/bme280.h>

#include <display/display.h>
#include <display/gfx/gfx.h>
#include <display/ssd1327/ssd1327_config.h>

#include <input/encoders/ky40.h>

static DHT_Config_t  hkDHT11;
static DHT_Config_t  hkDHT22;

static UART_Config_t hkUART0;
static I2C_Config_t  hkI2C0;
static I2C_Config_t  hkI2C1;

static PMS5003_Config_t hkPMS5003;
static DS18B20_Config_t hkDS18B20;
static SGP30_Config_t   hkSGP30;

static BME280_DataPacket_t hkBME280_0_Data;
static BME280_Config_t  hkBME280_0;
static BME280_Config_t  hkBME280_1;

static mutex_t hkI2C0_Mutex;
static mutex_t hkI2C1_Mutex;


queue_t displayDataQueue;
extern void hkDisplayLoop();

static bool DHT_Timer_ISR(struct repeating_timer* t);
static bool PMS5003_Timer_ISR(struct repeating_timer* t);
static bool DS18B20_Timer_ISR(struct repeating_timer* t);
static i64 SGP30_Read_Callback(alarm_id_t id, void* data);
static bool SGP30_Timer_ISR(struct repeating_timer* t);
static i64 SGP30_Baseline_Callback(alarm_id_t id, void* data);
static bool SGP30_Baseline_Timer_ISR(struct repeating_timer* t);

static i64 BME280_0_Read_Callback(alarm_id_t id, void* data);
static i64 BME280_1_Read_Callback(alarm_id_t id, void* data);
static bool BME280_0_Timer_ISR(struct repeating_timer* t);
static bool BME280_1_Timer_ISR(struct repeating_timer* t);


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
    hkUART0.uart     = hkUART;
    hkUART0.tx       = hkUART_TX;
    hkUART0.rx       = hkUART_RX;
    hkUART0.baudrate = hkUART_BAUDRATE;
    hkUART0.data     = hkUART_RawData;
    hkUART0.length   = sizeof(hkUART_RawData);
    hkUART0.status   = UART_INIT;
    hkUART0.packetSize = 32;   // How many bytes will be read from a single transmission
    UART_Init(&hkUART0);

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
    hkDS18B20.state       = DS18B20_IDLE;
    hkDS18B20.convertStartTime = 0;
    DS18B20_SetResolution(&ow0, 11);

    DS18B20_DataPacket_t hkDS18B20_Data = {
        .temperature = 0.0f,
        .address = 0,
        .jsonify = DS18B20_Jsonify
    };

    // static DHT_Config_t hkDHT22;
    static u8 hkDHT22_RawData[5];
    hkDHT22.gpio   = hkDHT_PIN;
    hkDHT22.pio    = hkDHT_PIO;
    hkDHT22.sm     = hkDHT_PIO_SM;
    hkDHT22.queue  = NULL;
    hkDHT22.status = DHT_INIT;
    hkDHT22.type   = DHT_22;
    hkDHT22.data   = hkDHT22_RawData;
    hkDHT22.length = sizeof(hkDHT22_RawData);
    DHT_Init(&hkDHT22);

    DHT_DataPacket_t hkDHT22_Data = {
        .humidity         = 0.0f,
        .absoluteHumidity = 0.0f,
        .temperature      = 0.0f,
        .dewPoint         = 0.0f,
        .jsonify = DHT_Jsonify
    };

    static u8 hkSGP30_RawData[6];
    hkSGP30.rawData = hkSGP30_RawData;
    hkSGP30.length  = sizeof(hkSGP30_RawData);
    hkSGP30.status  = SGP30_INIT;
    hkSGP30.eco2Baseline = 0;
    hkSGP30.tvocBaseline = 0;
    SGP30_Init(&hkI2C0, &hkSGP30);

    SGP30_DataPacket_t hkSGP30_Data = {
        .eco2 = 0,
        .tvoc = 0,
        .jsonify = SGP30_Jsonify
    };


    static u8 hkBME280_0_RawData[8];
    hkBME280_0.rawData = hkBME280_0_RawData;
    hkBME280_0.length  = sizeof(hkBME280_0_RawData);
    hkBME280_0.status  = BME280_INIT;
    hkBME280_0.address = hkBME280_ADDRESS;
    hkBME280_0.humiditySampling = 0x02;
    // hkBME280_0.iirCoefficient   = 0x08;
    hkBME280_0.iirCoefficient = 0xA0;
    // hkBME280_0.tempAndPressureMode = 0x49;
    hkBME280_0.tempAndPressureMode = 0x4B;
    BME280_Init(&hkI2C0, &hkBME280_0);

    hkBME280_0_Data.humidity = 0.0f;
    hkBME280_0_Data.absoluteHumidity = 0.0f;
    hkBME280_0_Data.pressure    = 0.0f;
    hkBME280_0_Data.temperature = 2.137f;
    hkBME280_0_Data.dewPoint    = 0.0f;
    hkBME280_0_Data.jsonify = BME280_Jsonify;


    static u8 hkBME280_1_RawData[8];
    hkBME280_1.rawData = hkBME280_1_RawData;
    hkBME280_1.length  = sizeof(hkBME280_1_RawData);
    hkBME280_1.status  = BME280_INIT;
    hkBME280_1.address = hkBME280_ADDRESS;
    hkBME280_1.humiditySampling = 0x02;
    // hkBME280_0.iirCoefficient   = 0x08;
    hkBME280_1.iirCoefficient = 0xA0;
    // hkBME280_0.tempAndPressureMode = 0x49;
    hkBME280_1.tempAndPressureMode = 0x4B;
    BME280_Init(&hkI2C1, &hkBME280_1);

    BME280_DataPacket_t hkBME280_1_Data = {
        .humidity = 0.0f,
        .pressure = 0.0f,
        .temperature = 2.137f,
        .jsonify = BME280_Jsonify
    };


    struct repeating_timer hkDHT22_Timer;
    struct repeating_timer hkPMS5003_Timer;
    struct repeating_timer hkDS18B20_Timer;
    struct repeating_timer hkSGP30_Timer;
    struct repeating_timer hkSGP30_Baseline_Timer;

    struct repeating_timer hkBME280_0_Timer;
    struct repeating_timer hkBME280_1_Timer;

    add_repeating_timer_ms(-2000, DHT_Timer_ISR, NULL, &hkDHT22_Timer);
    add_repeating_timer_ms(-2300, PMS5003_Timer_ISR, NULL, &hkPMS5003_Timer);
    // add_repeating_timer_ms(-100, DS18B20_Timer_ISR, NULL, &hkDS18B20_Timer);
    
    add_repeating_timer_ms(-1000, SGP30_Timer_ISR, NULL, &hkSGP30_Timer);
    // add_repeating_timer_ms(-1000, SGP30_Baseline_Timer_ISR, NULL, &hkSGP30_Baseline_Timer);
    
    add_repeating_timer_ms(-1200, BME280_0_Timer_ISR, NULL, &hkBME280_0_Timer);
    add_repeating_timer_ms(-1600, BME280_1_Timer_ISR, NULL, &hkBME280_1_Timer);


    queue_init(&displayDataQueue, sizeof(Sensors_DataPacket_t), 2);
    multicore_launch_core1(hkDisplayLoop);
    HINFO("[CORE1]: Core started for display rendering");

    BME280_DataPacket_t hkBME280_Average[2] = {hkBME280_0_Data, hkBME280_1_Data};
    size_t hkBME280_SensorsCount = sizeof(hkBME280_Average) / sizeof(hkBME280_Average[0]);

    Average_DataPacket_t hkTempAverage[4];
    size_t hkTempSensorsCount = sizeof(hkTempAverage) / sizeof(hkTempAverage[0]);

    Average_DataPacket_t hkDewPointAverage[3];
    size_t hkDewPointSensorsCount = sizeof(hkDewPointAverage) / sizeof(hkDewPointAverage[0]);

    Average_DataPacket_t hkHumidAverage[3];
    size_t hkHumidSensorsCount = sizeof(hkHumidAverage) / sizeof(hkHumidAverage[0]);

    Average_DataPacket_t hkAbsHumidAverage[3];
    size_t hkAbsHumidSensorsCount = sizeof(hkAbsHumidAverage) / sizeof(hkAbsHumidAverage[0]);

    while(FOREVER) {
        // DS18B20_ReadAndProcess(&ow0, &hkDS18B20, &hkDS18B20_Data);
        // HDEBUG("[DS18B20] TEMP: %.2f*C", hkDS18B20_Data.temperature);
        // char* hkDS18B20_Json = hkDS18B20_Data.jsonify(&hkDS18B20_Data);

        DS18B20_Read(&hkDS18B20);
        DS18B20_Tick(&ow0, &hkDS18B20, &hkDS18B20_Data);
        if(hkDS18B20_Data.status == DS18B20_DATA_READY) {
            hkTempAverage[0].value  = hkDS18B20_Data.temperature;
            hkTempAverage[0].weight = 10;

            HDEBUG("[DS18B20] TEMP: %.2f*C", hkDS18B20_Data.temperature);
            char* hkDS18B20_Json = hkDS18B20_Data.jsonify(&hkDS18B20_Data);
            hkDS18B20_Data.status = DS18B20_DATA_PROCESSED;
        }

        if(hkDHT22.status == DHT_READ_SUCCESS) {
            DHT_ProcessData(&hkDHT22, &hkDHT22_Data);
            
            hkTempAverage[1].value  = hkDHT22_Data.temperature;
            hkTempAverage[1].weight = 7;

            hkDewPointAverage[0].value  = hkDHT22_Data.dewPoint;
            hkDewPointAverage[0].weight = 7; 

            hkHumidAverage[0].value  = hkDHT22_Data.humidity;
            hkHumidAverage[0].weight = 7;

            hkAbsHumidAverage[0].value  = hkDHT22_Data.absoluteHumidity;
            hkAbsHumidAverage[0].weight = 7;

            HDEBUG("[ DHT22 ] TEMP: %.2f*C", hkDHT22_Data.temperature);
            HDEBUG("[ DHT22 ] DP  : %.2f*C", hkDHT22_Data.dewPoint);
            HDEBUG("[ DHT22 ] HUMI: %.2f %RH", hkDHT22_Data.humidity);
            HDEBUG("[ DHT22 ] HUMI: %.2f %AH", hkDHT22_Data.absoluteHumidity);
            char* hkDHT_Json = hkDHT22_Data.jsonify(&hkDHT22_Data);
        }

        if(hkUART0.status == UART_DATA_RX_SUCCESS) {
            #if hkBME280_USE_SENSOR && hkPMS5003_HUMID_COMPENSATION && hkPMS5003_USE_BME280
                PMS5003_ProcessDataHumidCompensation(&hkPMS5003, &hkPMS5003_Data, &hkBME280_0_Data);
            #elif hkDHT_USE_SENSOR && hkPMS5003_HUMID_COMPENSATION && hkPMS5003_USE_DHT
                PMS5003_ProcessDataHumidCompensation(&hkPMS5003, &hkPMS5003_Data, &hkDHT_Data);
            #else
                PMS5003_ProcessData(&hkPMS5003, &hkPMS5003_Data);
            #endif

            HDEBUG("[PMS5003] PM   1: %u ug/m3"  , hkPMS5003_Data.pm1);
            HDEBUG("[PMS5003] PM 2.5: %u ug/m3"  , hkPMS5003_Data.pm2_5);
            HDEBUG("[PMS5003] PM  10: %u ug/m3", hkPMS5003_Data.pm10);
            char* hkPMS5003_Json = hkPMS5003_Data.jsonify(&hkPMS5003_Data);
        }

        if(hkSGP30.status == SGP30_READY) {
            SGP30_ProcessData(&hkSGP30, &hkSGP30_Data);
            HDEBUG("[ SGP30 ] eCO2: %u ppm", hkSGP30_Data.eco2);
            HDEBUG("[ SGP30 ] TVOC: %u ppb", hkSGP30_Data.tvoc);
            char* hkSGP30_Json = hkSGP30_Data.jsonify(&hkSGP30_Data);
        }

        if(hkSGP30.status == SGP30_BASELINE_READY) {
            SGP30_ProcessBaseline(&hkSGP30);
            HDEBUG("[ SGP30 ] BeCO2: %u", hkSGP30.eco2Baseline);
            HDEBUG("[ SGP30 ] BTVOC: %u", hkSGP30.tvocBaseline);
        }

        if(hkBME280_0.status == BME280_READ_SUCCESS) {
            BME280_ProcessData(&hkBME280_0, &hkBME280_0_Data);
            hkBME280_Average[0] = hkBME280_0_Data;
            hkTempAverage[2].value  = hkBME280_0_Data.temperature;
            hkTempAverage[2].weight = 10;

            hkDewPointAverage[1].value  = hkBME280_0_Data.dewPoint;
            hkDewPointAverage[1].weight = 10; 

            hkHumidAverage[1].value  = hkBME280_0_Data.humidity;
            hkHumidAverage[1].weight = 10;
            
            hkAbsHumidAverage[1].value  = hkBME280_0_Data.absoluteHumidity;
            hkAbsHumidAverage[1].weight = 10;
            
            HDEBUG("[BME280 ] TEMP: %.2f*C", hkBME280_0_Data.temperature);
            HDEBUG("[BME280 ] DP  : %.2f*C", hkBME280_0_Data.dewPoint);
            HDEBUG("[BME280 ] HUMI: %.2f %RH", hkBME280_0_Data.humidity);
            HDEBUG("[BME280 ] HUMI: %.2f %AH", hkBME280_0_Data.absoluteHumidity);
            HDEBUG("[BME280 ] PRESS: %.1f hPa", hkBME280_0_Data.pressure);
            
            char* hkBME280_Json = hkBME280_0_Data.jsonify(&hkBME280_0_Data);
        }

        if(hkBME280_1.status == BME280_READ_SUCCESS) {
            BME280_ProcessData(&hkBME280_1, &hkBME280_1_Data);
            hkBME280_Average[1] = hkBME280_1_Data;
            hkTempAverage[3].value  = hkBME280_1_Data.temperature;
            hkTempAverage[3].weight = 10;

            hkDewPointAverage[2].value  = hkBME280_1_Data.dewPoint;
            hkDewPointAverage[2].weight = 10; 

            hkHumidAverage[2].value  = hkBME280_1_Data.humidity;
            hkHumidAverage[2].weight = 10;

            hkAbsHumidAverage[2].value  = hkBME280_1_Data.absoluteHumidity;
            hkAbsHumidAverage[2].weight = 10;

            HDEBUG("[BME281 ] TEMP: %.2f*C", hkBME280_1_Data.temperature);
            HDEBUG("[BME281 ] HUMI: %.2f%%", hkBME280_1_Data.humidity);
            HDEBUG("[BME281 ] PRESS: %.1f hPa", hkBME280_1_Data.pressure);
            
            char* hkBME280_Json = hkBME280_1_Data.jsonify(&hkBME280_1_Data);
        }

        if(hkSSD1327.status == DISP_DRAW) {
            Sensors_DataPacket_t dataToDisplay;

            dataToDisplay.dht     = hkDHT22_Data;
            dataToDisplay.ds18b20 = hkDS18B20_Data;
            dataToDisplay.pms5003 = hkPMS5003_Data;
            dataToDisplay.sgp30   = hkSGP30_Data;
            dataToDisplay.bme280  = BME280_AverageData(hkBME280_Average, hkBME280_SensorsCount);
            dataToDisplay.temperature = hkWeightedAverage(hkTempAverage, hkTempSensorsCount);
            dataToDisplay.dewPoint    = hkWeightedAverage(hkDewPointAverage, hkDewPointSensorsCount);
            dataToDisplay.humidity    = hkWeightedAverage(hkHumidAverage, hkHumidSensorsCount);
            dataToDisplay.absHumidity = hkWeightedAverage(hkAbsHumidAverage, hkAbsHumidSensorsCount);
            
            queue_try_add(&displayDataQueue, &dataToDisplay);
        }

        printf("\n");
        // sleep_ms(1000);
    }
}





static bool DHT_Timer_ISR(struct repeating_timer* t) {
    DHT_Read(&hkDHT22);
    return true;
}

static bool PMS5003_Timer_ISR(struct repeating_timer* t) {
    PMS5003_Read(&hkUART0, &hkPMS5003);
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
    #if hkBME280_USE_SENSOR && hkSGP30_HUMID_COMPENSATION && hkSGP30_USE_BME280
        SGP30_InitReadHumidCompensation(&hkI2C0, &hkSGP30, &hkBME280_0_Data);
    #elif hkDHT_USE_SENSOR && hkSGP30_HUMID_COMPENSATION && hkSGP30_USE_DHT
        SGP30_InitReadHumidCompensation(&hkI2C0, &hkSGP30, &hkBME280_0_Data);
    #else
        SGP30_InitRead(&hkI2C0, &hkSGP30);
    #endif

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


static i64 BME280_0_Read_Callback(alarm_id_t id, void* data) {
    BME280_Read(&hkI2C0, &hkBME280_0);
    return false;
}

static bool BME280_0_Timer_ISR(struct repeating_timer* t) {
    // BME280_InitRead(&hkI2C0, &hkBME280_0);
    // add_alarm_in_ms(15, BME280_0_Read_Callback, NULL, true);
    BME280_Read(&hkI2C0, &hkBME280_0);
    return true;
}

static bool BME280_1_Timer_ISR(struct repeating_timer* t) {
    BME280_Read(&hkI2C0, &hkBME280_1);
    return true;
}









//         if(hkDHT22.status == DHT_READ_INIT) {
//             hkDHT22.status = DHT_INIT;
//             DHT11_Read(&hkDHT22);
//         }
//         if(hkPMS5003.status == PMS5003_READ_INIT) {
//             hkPMS5003.status = PMS5003_INIT;
//             PMS5003_Read(&hkUART0, &hkPMS5003);
//         }
//         if(hkDS18B20.state == DS18B20_READ_INIT) {
//             hkDS18B20.state = DS18B20_IDLE;
//             DS18B20_Read(&hkDS18B20);
//         }
//         if(hkSGP30.status == SGP30_READ_INIT) {
//             hkSGP30.status = SGP30_INIT;
//             SGP30_InitRead(&hkI2C0, &hkSGP30);
//         }
//         if(hkSGP30.status == SGP30_READ_READY) {
//             hkSGP30.status = SGP30_INIT;
//             SGP30_Read(&hkI2C0, &hkSGP30);
//         }
//         if(hkSGP30.status == SGP30_READ_BASELINE_INIT) {
//             hkSGP30.status = SGP30_INIT;
//             SGP30_InitGetBaseline(&hkI2C0, &hkSGP30);
//         }
//         if(hkSGP30.status == SGP30_READ_BASELINE_READY) {
//             hkSGP30.status = SGP30_INIT;
//             SGP30_GetBaseline(&hkI2C0, &hkSGP30);
//         }
//         if(hkBME280_0.status == BME280_READ_READY) {
//             hkBME280_0.status = BME280_INIT;
//             BME280_Read(&hkI2C0, &hkBME280_0);
//         }


//         printf("\n");
//         sleep_ms(1000);
//     }
// }


// static bool DHT11_Timer_ISR(struct repeating_timer* t) {
//     hkDHT22.status = DHT_READ_INIT;
//     return true;
// }

// static bool PMS5003_Timer_ISR(struct repeating_timer* t) {
//     hkPMS5003.status = PMS5003_READ_INIT;
//     return true;
// }

// static bool DS18B20_Timer_ISR(struct repeating_timer* t) {
//     hkDS18B20.state = DS18B20_READ_INIT;
//     return true;
// }

// static i64 SGP30_Read_Callback(alarm_id_t id, void* data) {
//     hkSGP30.status = SGP30_READ_READY;
//     return false;
// }

// static bool SGP30_Timer_ISR(struct repeating_timer* t) {
//     hkSGP30.status = SGP30_READ_INIT;
//     add_alarm_in_ms(15, SGP30_Read_Callback, NULL, true);
//     return true;
// }

// static i64 SGP30_Baseline_Callback(alarm_id_t id, void* data) {
//     hkSGP30.status = SGP30_READ_BASELINE_READY;
//     return false;
// }

// static bool SGP30_Baseline_Timer_ISR(struct repeating_timer* t) {
//     hkSGP30.status = SGP30_READ_BASELINE_INIT;
//     add_alarm_in_ms(15, SGP30_Baseline_Callback, NULL, true);
//     return true;
// }

// // static i64 BME280_0_Read_Callback(alarm_id_t id, void* data) {
// //     BME280_Read(&hkI2C0, &hkBME280_0);
// //     return false;
// // }

// static bool BME280_0_Timer_ISR(struct repeating_timer* t) {
//     // BME280_InitRead(&hkI2C0, &hkBME280_0);
//     // add_alarm_in_ms(15, BME280_0_Read_Callback, NULL, true);
//     hkBME280_0.status = BME280_READ_READY;
//     return true;
// }