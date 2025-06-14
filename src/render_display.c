// Newlib
#include <stdio.h>

// Pico SDK
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>

// hkRTOS
#include <defines.h>

#include <sensors/sensors.h>
#include <sensors/dht11/dht11.h>
#include <sensors/ds18b20/ds18b20.h>
#include <sensors/pms5003/pms5003.h>
#include <sensors/sgp30/sgp30.h>
#include <sensors/bme280/bme280.h>

#include <display/display.h>
#include <display/gfx/gfx.h>
#include <display/ssd1327/ssd1327_config.h>

extern queue_t displayDataQueue;

void hkDisplayLoop() {
    Sensors_DataPacket_t data;

    static u8 DS18B20_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DS18B20_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
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

    static u8 SGP30_ECO2_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t SGP30_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 73,        // Start Y point
        .height = 50,   // End Y point   
        .minVal = 300.0,
        .maxVal = 1500.0,
        .colour = 10,
        .borderColour = 10,
        .legendColour = 4,
        .cursorX = 0,
        .history = SGP30_ECO2_GraphHistory,
        .length  = sizeof(SGP30_ECO2_GraphHistory)
    }; hkGraphInit(&SGP30_Graph);

    hkClearBuffer();
    hkGraphDrawAxes(&DS18B20_Graph);
    // hkGraphDrawAxes(&PMS5003_PM2_5_Graph);
    hkGraphDrawAxes(&SGP30_Graph);
    hkDisplay();


    while(FOREVER) {
        queue_remove_blocking(&displayDataQueue, &data);

        hkClearBuffer();

        hkGraphAddDataPoint(&DS18B20_Graph, data.ds18b20.temperature);
        // hkGraphAddDataPoint(&PMS5003_PM2_5_Graph, (f32)hkPMS5003_Data.pm2_5);
        hkGraphAddDataPoint(&SGP30_Graph, data.sgp30.eco2);
        
        hkGraphDrawAxes(&DS18B20_Graph);
        hkGraphDrawLegend(&DS18B20_Graph, "TEMP (C)");

        // hkGraphDrawAxes(&PMS5003_PM2_5_Graph);
        // hkGraphDrawLegend(&PMS5003_PM2_5_Graph, "PM2.5");

        hkGraphDrawAxes(&SGP30_Graph);
        hkGraphDrawLegend(&SGP30_Graph, "eCO2");

        hkGraphDraw(&DS18B20_Graph);
        // hkGraphDraw(&PMS5003_PM2_5_Graph);
        hkGraphDraw(&SGP30_Graph);

        hkDisplay();
    }
}