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


QueueSetHandle_t xSensorQueueSet;
QueueHandle_t xBME280_0_DataQueue;
QueueHandle_t xBME280_1_DataQueue;
QueueHandle_t xSGP30_0_DataQueue;
QueueHandle_t xPMS5003_0_DataQueue;
QueueHandle_t xDS18B20_0_DataQueue;
QueueHandle_t xDHT20_0_DataQueue;
QueueHandle_t xDHT20_1_DataQueue;
QueueHandle_t xBMP180_0_DataQueue;


extern void vDisplayTask(void* pvParameters);


void main(void) {
    stdio_init_all();
    DMA_Master_Init();

    // ************************************************************************
    // = I2C ===
    // ------------------------------------------------------------------------
    static mutex_t hkI2C0_Mutex;
    static mutex_t hkI2C1_Mutex;

    mutex_init(&hkI2C0_Mutex);
    static I2C_Config_t hkI2C0 = {
        .i2c   = hkI2C_ONE,
        .scl   = hkI2C_SCL_ONE,
        .sda   = hkI2C_SDA_ONE,
        .speed = hkI2C_SPEED_ONE,
        .mutex = &hkI2C0_Mutex 
    }; I2C_Init(&hkI2C0);

    mutex_init(&hkI2C1_Mutex);
    static I2C_Config_t hkI2C1 = {
        .i2c   = hkI2C_TWO,
        .scl   = hkI2C_SCL_TWO,
        .sda   = hkI2C_SDA_TWO,
        .speed = hkI2C_SPEED_TWO,
        .mutex = &hkI2C1_Mutex 
    }; I2C_Init(&hkI2C1);
    // ------------------------------------------------------------------------


    // ************************************************************************
    // = UART ===
    // ------------------------------------------------------------------------
    static u8 hkUART0_RawData[32];
    static UART_Config_t hkUART0 = {
        .uart     = hkUART,
        .tx       = hkUART_TX,
        .rx       = hkUART_RX,
        .baudrate = hkUART_BAUDRATE,
        .data     = hkUART0_RawData,
        .length   = sizeof(hkUART0_RawData),
        .status   = UART_INIT,
        .packetSize = 32    // How many bytes will be read from a single transmission
    }; 
    
    hkUART0.dmaSemaphore = xSemaphoreCreateBinary();
    if(hkUART0.dmaSemaphore == NULL) HFATAL("main(): Failed to create UART DMA semaphore!");
    UART_Init(&hkUART0);
    // ------------------------------------------------------------------------

    
    // ************************************************************************
    // = OneWire ===
    // ------------------------------------------------------------------------
    static OneWire_Config_t hkOneWire0 = {
        .gpio    = hkOW_PIN,
        .pio     = hkOW_PIO,
        .sm      = hkOW_PIO_SM,
        .status  = ONEW_INIT,
        .bitMode = 8
    }; OneWire_Init(&hkOneWire0);
    // ------------------------------------------------------------------------



    // ************************************************************************
    // = BME280 ===
    // ------------------------------------------------------------------------
    static u8 hkBME280_0_RawData[8];
    static BME280_Config_t hkBME280_0 = {
        .address = hkBME280_ADDRESS,
        .status  = BME280_INIT,
        .rawData = hkBME280_0_RawData,
        .length  = sizeof(hkBME280_0_RawData),
        .humiditySampling = 0x02,
        .iirCoefficient   = 0xA0,
        .tempAndPressureMode = 0x4B
    };

    xBME280_0_DataQueue = xQueueCreate(1, sizeof(BME280_DataPacket_t));
    if(xBME280_0_DataQueue == NULL) HFATAL("main(): Failed to create BME280_0 data queue!");
    hkBME280_0.queue = xBME280_0_DataQueue;

    static BME280_DataPacket_t hkBME280_0_Data = {
        .humidity = 0.0f,
        .pressure = 0.0f,
        .temperature = 0.0f,
        .jsonify = BME280_Jsonify
    };

    static BME280_TaskParams_t hkBME280_0_TaskParams = {
        .i2c    = &hkI2C0,
        .bme280 = &hkBME280_0,
        .data   = &hkBME280_0_Data
    };


    static u8 hkBME280_1_RawData[8];
    static BME280_Config_t hkBME280_1 = {
        .address = hkBME280_ADDRESS,
        .status  = BME280_INIT,
        .rawData = hkBME280_1_RawData,
        .length  = sizeof(hkBME280_1_RawData),
        .humiditySampling = 0x02,
        .iirCoefficient   = 0xA0,
        .tempAndPressureMode = 0x4B
    };

    xBME280_1_DataQueue = xQueueCreate(1, sizeof(BME280_DataPacket_t));
    if(xBME280_1_DataQueue == NULL) HFATAL("main(): Failed to create BME280_1 data queue!");
    hkBME280_1.queue = xBME280_1_DataQueue;

    static BME280_DataPacket_t hkBME280_1_Data = {
        .humidity = 0.0f,
        .pressure = 0.0f,
        .temperature = 0.0f,
        .jsonify = BME280_Jsonify
    };

    static BME280_TaskParams_t hkBME280_1_TaskParams = {
        .i2c    = &hkI2C1,
        .bme280 = &hkBME280_1,
        .data   = &hkBME280_1_Data
    };
    // ------------------------------------------------------------------------


    // ************************************************************************
    // = SGP30 ===
    // ------------------------------------------------------------------------
    static u8 hkSGP30_0_RawData[6];
    static SGP30_Config_t hkSGP30_0 = {
        .address = hkSGP30_ADDRESS,
        .rawData = hkSGP30_0_RawData,
        .length  = sizeof(hkSGP30_0_RawData),
        .status  = SGP30_INIT,
        .eco2Baseline = 0,
        .tvocBaseline = 0
    };

    xSGP30_0_DataQueue = xQueueCreate(1, sizeof(SGP30_DataPacket_t));
    if(xSGP30_0_DataQueue == NULL) HFATAL("main(): Failed to create SGP30_0 data queue!");
    hkSGP30_0.queue = xSGP30_0_DataQueue;

    static SGP30_DataPacket_t hkSGP30_0_Data = {
        .eco2 = 0,
        .tvoc = 0,
        .jsonify = SGP30_Jsonify
    };

    static SGP30_TaskParams_t hkSGP30_0_TaskParams = {
        .i2c   = &hkI2C0,
        .sgp30 = &hkSGP30_0,
        .data  = &hkSGP30_0_Data,
        .humidSensor = NULL
    };
    // ------------------------------------------------------------------------


    // ************************************************************************
    // = PMS5003 ===
    // ------------------------------------------------------------------------
    static u8 hkPMS5003_0_RawData[32];
    static PMS5003_Config_t hkPMS5003_0 = {
        .rawData = hkPMS5003_0_RawData,
        .length  = sizeof(hkPMS5003_0_RawData)
    };

    xPMS5003_0_DataQueue = xQueueCreate(1, sizeof(PMS5003_DataPacket_t));
    if(xPMS5003_0_DataQueue == NULL) HFATAL("main(): Failed to create PMS5003_0 data queue!");
    hkPMS5003_0.queue = xPMS5003_0_DataQueue;

    static PMS5003_DataPacket_t hkPMS5003_0_Data = {
        .pm1   = 0,
        .pm2_5 = 0,
        .pm10  = 0,
        .jsonify = PMS5003_Jsonify
    };

    static PMS5003_TaskParams_t hkPMS5003_0_TaskParams = {
        .uart    = &hkUART0,
        .pms5003 = &hkPMS5003_0,
        .data    = &hkPMS5003_0_Data,
        .humidSensor = NULL 
    };
    // ------------------------------------------------------------------------


    // ************************************************************************
    // = DS18B20 ===
    // ------------------------------------------------------------------------
    static u8 hkDS18B20_0_RawData[9];
    static DS18B20_Config_t hkDS18B20_0 = {
        // .address  = 0x2836C7BE000000B6
        .address     = ONEW_SKIP_ROM,
        .data        = hkDS18B20_0_RawData,
        .length      = sizeof(hkDS18B20_0_RawData),
        .temperature = 0.0f,
        .dataCount   = 0,
        .state       = DS18B20_IDLE,
        .convertStartTime = 0,
        .resolution  = 12
    };

    xDS18B20_0_DataQueue = xQueueCreate(1, sizeof(BME280_DataPacket_t));
    if(xDS18B20_0_DataQueue == NULL) HFATAL("main(): Failed to create DS18B20_0 data queue!");
    hkDS18B20_0.queue = xDS18B20_0_DataQueue;

    static DS18B20_DataPacket_t hkDS18B20_0_Data = {
        .temperature = 0.0f,
        .address = 0,
        .jsonify = DS18B20_Jsonify
    };

    static DS18B20_TaskParams_t hkDS18B20_0_TaskParams = {
        .ow      = &hkOneWire0,
        .ds18b20 = &hkDS18B20_0,
        .data    = &hkDS18B20_0_Data
    };
    // ------------------------------------------------------------------------
    


    // ************************************************************************
    // = KY40 ===
    // ------------------------------------------------------------------------
    const KY40_Config_t hkKY40_Encoders[hkKY40_ENCODERS_COUNT] = {
        {.clk = hkKY40_CLK_PIN, .dt = hkKY40_DT_PIN, .btn = hkKY40_BTN_PIN},
        // {.clk = 28            , .dt = 27           , .btn = 26}
    }; KY40_InitAll(hkKY40_Encoders);
    // ------------------------------------------------------------------------



    // ************************************************************************
    // = RTOS ===
    // ------------------------------------------------------------------------
    xSensorQueueSet = xQueueCreateSet(5);   // Number of sensors


    // - BME280 ---
    xQueueAddToSet(xBME280_0_DataQueue, xSensorQueueSet);
    xTaskCreate(vBME280_Task, "BME280_0_Task", (configMINIMAL_STACK_SIZE + 256), &hkBME280_0_TaskParams, 1, NULL);

    xQueueAddToSet(xBME280_1_DataQueue, xSensorQueueSet);
    xTaskCreate(vBME280_Task, "BME280_1_Task", (configMINIMAL_STACK_SIZE + 256), &hkBME280_1_TaskParams, 1, NULL);
    // ------------

    // - SGP30 ---
    xQueueAddToSet(xSGP30_0_DataQueue, xSensorQueueSet);
    xTaskCreate(vSGP30_Task, "SGP30_0_Task", (configMINIMAL_STACK_SIZE + 256), &hkSGP30_0_TaskParams, 1, NULL);
    // ------------

    // - DS18B20 ---
    xQueueAddToSet(xDS18B20_0_DataQueue, xSensorQueueSet);
    xTaskCreate(vDS18B20_Task, "DS18B20_0_Task", (configMINIMAL_STACK_SIZE + 256), &hkDS18B20_0_TaskParams, 1, NULL);
    // ------------

    // - PMS5003 ---
    xQueueAddToSet(xPMS5003_0_DataQueue, xSensorQueueSet);
    xTaskCreate(vPMS5003_Task, "PMS5003_0_Task", (configMINIMAL_STACK_SIZE + 256), &hkPMS5003_0_TaskParams, 1, NULL);
    // ------------


    xTaskCreate(vDisplayTask, "Display_Task", (configMINIMAL_STACK_SIZE + 256), NULL, 2, NULL);

    HINFO("Starting RTOS scheduler");
    vTaskStartScheduler();

    while(1);
    // ------------------------------------------------------------------------
}
