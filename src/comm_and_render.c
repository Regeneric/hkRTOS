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

#include <input/encoders/ky40.h>

static KY40_Config_t encoders[hkKY40_ENCODERS_COUNT];

extern queue_t displayDataQueue;

void hkDisplayLoop() {
    Sensors_DataPacket_t data;

    static u8 DS18B20_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DS18B20_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 110,  // End Y point  
        .minVal = -15.0,
        .maxVal = 50.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = DS18B20_GraphHistory,
        .length  = sizeof(DS18B20_GraphHistory)
    }; hkGraphInit(&DS18B20_Graph);


    static u8 SGP30_TVOC_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t SGP30_TVOC_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point   
        .minVal = 0.0,
        .maxVal = 500.0,
        .colour = 10,
        .borderColour = 10,
        .legendColour = 4,
        .cursorX = 0,
        .history = SGP30_TVOC_GraphHistory,
        .length  = sizeof(SGP30_TVOC_GraphHistory)
    }; hkGraphInit(&SGP30_TVOC_Graph);

    static u8 SGP30_ECO2_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t SGP30_ECO2_Graph = {
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
    }; hkGraphInit(&SGP30_ECO2_Graph);


    static u8 PMS5003_1_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t PMS5003_1_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = 0.0,
        .maxVal = 150.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = PMS5003_1_GraphHistory,
        .length  = sizeof(PMS5003_1_GraphHistory)
    }; hkGraphInit(&PMS5003_1_Graph);

    static u8 PMS5003_2_5_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t PMS5003_2_5_Graph = {
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
    }; hkGraphInit(&PMS5003_2_5_Graph);


    static u8 BME280_Temp_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t BME280_Temp_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = 0.0,
        .maxVal = 150.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = BME280_Temp_GraphHistory,
        .length  = sizeof(BME280_Temp_GraphHistory)
    }; hkGraphInit(&BME280_Temp_Graph);

    static u8 BME280_Humidity_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t BME280_Humidity_Graph = {
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
        .history = BME280_Humidity_GraphHistory,
        .length  = sizeof(BME280_Humidity_GraphHistory)
    }; hkGraphInit(&BME280_Humidity_Graph);


    static u8 DHT11_Temp_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DHT11_Humidity_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = 0.0,
        .maxVal = 150.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = DHT11_Temp_GraphHistory,
        .length  = sizeof(DHT11_Temp_GraphHistory)
    }; hkGraphInit(&DHT11_Humidity_Graph);

    static u8 DHT11_Humidity_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DHT11_Temp_Graph = {
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
        .history = DHT11_Humidity_GraphHistory,
        .length  = sizeof(DHT11_Humidity_GraphHistory)
    }; hkGraphInit(&DHT11_Temp_Graph);


    hkClearBuffer();
    hkDisplay();

    const KY40_Config_t KY40_Encoders[hkKY40_ENCODERS_COUNT] = {
        // {CLK, DT, BTN}
        {.clk = hkKY40_CLK_PIN, .dt = hkKY40_DT_PIN, .btn = hkKY40_BTN_PIN},
        // {.clk = 28            , .dt = 27           , .btn = 26}
    }; KY40_InitAll(KY40_Encoders);


    b8 dataFromCore0 = false;
    u8 lastPosition  = KY40_Position(0, hkKY40_GET);

    while(FOREVER) {
        // queue_remove_blocking(&displayDataQueue, &data);
        dataFromCore0 = queue_try_remove(&displayDataQueue, &data);
        // hkClearBuffer();

        // hkGraphAddDataPoint(&DS18B20_Graph, data.ds18b20.temperature);
        // hkGraphAddDataPoint(&SGP30_Graph, data.sgp30.eco2);
        
        // hkGraphDrawAxes(&DS18B20_Graph);
        // hkGraphDrawLegend(&DS18B20_Graph, "TEMP (C)");

        // hkGraphDrawAxes(&SGP30_Graph);
        // hkGraphDrawLegend(&SGP30_Graph, "eCO2");

        // hkGraphDraw(&DS18B20_Graph);
        // hkGraphDraw(&SGP30_Graph);

        // HDEBUG("POSITION: %d", KY40_Position(0, hkKY40_GET));

        if(dataFromCore0 || (KY40_Position(0, hkKY40_GET) != lastPosition)) {
            lastPosition = KY40_Position(0, hkKY40_GET);

            hkGraphAddDataPoint(&SGP30_TVOC_Graph, data.sgp30.tvoc);
            hkGraphAddDataPoint(&SGP30_ECO2_Graph, data.sgp30.eco2);
            hkGraphAddDataPoint(&PMS5003_1_Graph, (f32)data.pms5003.pm1);
            hkGraphAddDataPoint(&PMS5003_2_5_Graph, (f32)data.pms5003.pm2_5);
            hkGraphAddDataPoint(&DS18B20_Graph, data.ds18b20.temperature);
            hkGraphAddDataPoint(&BME280_Temp_Graph, data.bme280.temperature);
            hkGraphAddDataPoint(&BME280_Humidity_Graph, data.bme280.humidity);
            hkGraphAddDataPoint(&DHT11_Temp_Graph, data.dht.temperature);
            hkGraphAddDataPoint(&DHT11_Humidity_Graph, data.dht.humidity);

            switch(KY40_Position(0, hkKY40_GET)) {
                case 0:
                    hkClearBuffer();
                
                    hkGraphDrawAxes(&SGP30_TVOC_Graph);
                    hkGraphDrawLegend(&SGP30_TVOC_Graph, "TVOC (ppb)");

                    hkGraphDrawAxes(&SGP30_ECO2_Graph);
                    hkGraphDrawLegend(&SGP30_ECO2_Graph, "eCO2 (ppm)");

                    hkGraphDrawValue(&SGP30_TVOC_Graph, data.sgp30.tvoc, 0);
                    hkGraphDrawValue(&SGP30_ECO2_Graph, data.sgp30.eco2, 0);

                    hkGraphDraw(&SGP30_TVOC_Graph);
                    hkGraphDraw(&SGP30_ECO2_Graph);
                    break;
                case 1:
                    hkClearBuffer();

                    hkGraphDrawAxes(&PMS5003_1_Graph);
                    hkGraphDrawAxes(&PMS5003_2_5_Graph);

                    hkGraphDrawLegend(&PMS5003_1_Graph, "PM1 (ug/m3)");
                    hkGraphDrawLegend(&PMS5003_2_5_Graph, "PM2.5 (ug/m3)");

                    hkGraphDrawValue(&PMS5003_1_Graph, data.pms5003.pm1, 0);
                    hkGraphDrawValueFollow(&PMS5003_1_Graph, data.pms5003.pm1, 0);
                    
                    hkGraphDrawValue(&PMS5003_2_5_Graph, data.pms5003.pm2_5, 0);
                    hkGraphDrawValueFollow(&PMS5003_2_5_Graph, data.pms5003.pm2_5, 0);

                    hkGraphDraw(&PMS5003_1_Graph);
                    hkGraphDraw(&PMS5003_2_5_Graph);
                    break;
                case 2:
                    hkClearBuffer();

                    hkGraphDrawAxes(&DS18B20_Graph);
                    hkGraphDrawLegend(&DS18B20_Graph, "TEMP (C)");
                    hkGraphDrawValueFollow(&DS18B20_Graph, data.ds18b20.temperature, 1);

                    hkGraphDraw(&DS18B20_Graph);
                    break;
                case 3:
                    hkClearBuffer();

                    hkGraphDrawAxes(&BME280_Temp_Graph);
                    hkGraphDrawAxes(&BME280_Humidity_Graph);

                    hkGraphDrawLegend(&BME280_Temp_Graph, "TEMP (C)");
                    hkGraphDrawLegend(&BME280_Humidity_Graph, "HUMIDITY (%RH)");

                    hkGraphDrawValue(&BME280_Temp_Graph, data.bme280.temperature, 1);
                    hkGraphDrawValueFollow(&BME280_Temp_Graph, data.bme280.temperature, 1);
                    
                    hkGraphDrawValue(&BME280_Humidity_Graph, data.bme280.humidity, 1);
                    hkGraphDrawValueFollow(&BME280_Humidity_Graph, data.bme280.humidity, 1);

                    hkGraphDraw(&BME280_Temp_Graph);
                    hkGraphDraw(&BME280_Humidity_Graph);
                    break;
                case 4:
                    hkClearBuffer();

                    hkGraphDrawAxes(&DHT11_Temp_Graph);
                    hkGraphDrawAxes(&DHT11_Humidity_Graph);

                    hkGraphDrawLegend(&DHT11_Temp_Graph, "TEMP (C)");
                    hkGraphDrawLegend(&DHT11_Humidity_Graph, "HUMIDITY (%RH)");

                    hkGraphDrawValue(&DHT11_Temp_Graph, data.dht.temperature, 1);
                    hkGraphDrawValueFollow(&DHT11_Temp_Graph, data.dht.temperature, 1);
                    
                    hkGraphDrawValue(&DHT11_Humidity_Graph, data.dht.humidity, 1);
                    hkGraphDrawValueFollow(&DHT11_Humidity_Graph, data.dht.humidity, 1);

                    hkGraphDraw(&DHT11_Temp_Graph);
                    hkGraphDraw(&DHT11_Humidity_Graph);
                    break;
                case 5:
                    hkClearBuffer();

                    hkGraphDrawAxes(&BME280_Humidity_Graph);
                    hkGraphDrawAxes(&DHT11_Humidity_Graph);

                    hkGraphDrawLegend(&BME280_Humidity_Graph, "BME280 (%RH)");
                    hkGraphDrawLegend(&DHT11_Humidity_Graph, "DHT11 (%RH)");

                    hkGraphDrawValue(&BME280_Humidity_Graph, data.bme280.humidity, 1);
                    hkGraphDrawValueFollow(&BME280_Humidity_Graph, data.bme280.humidity, 1);
                    
                    hkGraphDrawValue(&DHT11_Humidity_Graph, data.dht.humidity, 1);
                    hkGraphDrawValueFollow(&DHT11_Humidity_Graph, data.dht.humidity, 1);

                    hkGraphDraw(&BME280_Humidity_Graph);
                    hkGraphDraw(&DHT11_Humidity_Graph);
                    break;
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                case 17:
                case 18:
                case 19:
                case 20:
                default: break;
            }
        }

        hkDisplay();
    }
}