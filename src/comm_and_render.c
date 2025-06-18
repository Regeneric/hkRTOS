// Newlib
#include <stdio.h>

// Pico SDK
#include <pico/stdlib.h>
#include <pico/stdio.h>
#include <pico/multicore.h>
#include <pico/util/queue.h>

// hkRTOS
#include <defines.h>

#include <core/hmean.h>

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

static KY40_Config_t encoders[hkKY40_ENCODERS_COUNT];

extern queue_t displayDataQueue;


struct repeating_timer timer;
const u32 tenMinutes = 10 * (60*1000);
vu8 screenSaver = false;

bool screenSaverTimer(struct repeating_timer* t) {
    screenSaver = true;
    return true;
}

static void resetScreeSaverTimer() {
    cancel_repeating_timer(&timer);
    add_repeating_timer_ms(-tenMinutes, screenSaverTimer, NULL, &timer);
    // add_repeating_timer_ms(-10, screenSaverTimer, NULL, &timer);
}


void hkDisplayLoop() {
    add_repeating_timer_ms(-tenMinutes, screenSaverTimer, NULL, &timer);
    // add_repeating_timer_ms(-10, screenSaverTimer, NULL, &timer);


    Sensors_DataPacket_t data;

    static u8 DS18B20_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DS18B20_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 110,  // End Y point  
        .minVal = -10.0,
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
        .colour = 15,
        .borderColour = 15,
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


    static u8 PMS5003_2_5_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t PMS5003_2_5_Graph = {
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
        .history = PMS5003_2_5_GraphHistory,
        .length  = sizeof(PMS5003_2_5_GraphHistory)
    }; hkGraphInit(&PMS5003_2_5_Graph);

    static u8 PMS5003_10_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t PMS5003_10_Graph = {
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
        .history = PMS5003_10_GraphHistory,
        .length  = sizeof(PMS5003_10_GraphHistory)
    }; hkGraphInit(&PMS5003_10_Graph);


    static u8 BME280_Temp_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t BME280_Temp_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = -10.0,
        .maxVal = 50.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = BME280_Temp_GraphHistory,
        .length  = sizeof(BME280_Temp_GraphHistory)
    }; hkGraphInit(&BME280_Temp_Graph);

    static u8 BME280_DewPoint_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t BME280_DewPoint_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 73,        // Start Y point
        .height = 50,   // End Y point   
        .minVal = -10.0,
        .maxVal = 50.0,
        .colour = 10,
        .borderColour = 10,
        .legendColour = 4,
        .cursorX = 0,
        .history = BME280_DewPoint_GraphHistory,
        .length  = sizeof(BME280_DewPoint_GraphHistory)
    }; hkGraphInit(&BME280_DewPoint_Graph);

    static u8 BME280_AbsoluteHumidity_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t BME280_AbsoluteHumidity_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = 0.0,
        .maxVal = 100.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = BME280_AbsoluteHumidity_GraphHistory,
        .length  = sizeof(BME280_AbsoluteHumidity_GraphHistory)
    }; hkGraphInit(&BME280_AbsoluteHumidity_Graph);

    static u8 BME280_RelativeHumidity_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t BME280_RelativeHumidity_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 73,        // Start Y point
        .height = 50,   // End Y point   
        .minVal = 0.0,
        .maxVal = 100.0,
        .colour = 10,
        .borderColour = 10,
        .legendColour = 4,
        .cursorX = 0,
        .history = BME280_RelativeHumidity_GraphHistory,
        .length  = sizeof(BME280_RelativeHumidity_GraphHistory)
    }; hkGraphInit(&BME280_RelativeHumidity_Graph);


    static u8 DHT11_RelativeHumidity_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DHT11_RelativeHumidity_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = 0.0,
        .maxVal = 100.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = DHT11_RelativeHumidity_GraphHistory,
        .length  = sizeof(DHT11_RelativeHumidity_GraphHistory)
    }; hkGraphInit(&DHT11_RelativeHumidity_Graph);

    static u8 DHT11_AbsoluteHumidity_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DHT11_AbsoluteHumidity_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 73,        // Start Y point
        .height = 50,   // End Y point   
        .minVal = 0.0,
        .maxVal = 100.0,
        .colour = 10,
        .borderColour = 10,
        .legendColour = 4,
        .cursorX = 0,
        .history = DHT11_AbsoluteHumidity_GraphHistory,
        .length  = sizeof(DHT11_AbsoluteHumidity_GraphHistory)
    }; hkGraphInit(&DHT11_AbsoluteHumidity_Graph);

    static u8 DHT11_DewPoint_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DHT11_DewPoint_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = -10.0,
        .maxVal = 50.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = DHT11_DewPoint_GraphHistory,
        .length  = sizeof(DHT11_DewPoint_GraphHistory)
    }; hkGraphInit(&DHT11_DewPoint_Graph);

    static u8 DHT11_Temp_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DHT11_Temp_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 73,        // Start Y point
        .height = 50,   // End Y point   
        .minVal = -10.0,
        .maxVal = 50.0,
        .colour = 10,
        .borderColour = 10,
        .legendColour = 4,
        .cursorX = 0,
        .history = DHT11_Temp_GraphHistory,
        .length  = sizeof(DHT11_Temp_GraphHistory)
    }; hkGraphInit(&DHT11_Temp_Graph);


    static u8 TempAverage_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t TempAverage_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = -10.0,
        .maxVal = 50.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = TempAverage_GraphHistory,
        .length  = sizeof(TempAverage_GraphHistory)
    }; hkGraphInit(&TempAverage_Graph);

    static u8 DewPointAverage_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t DewPointAverage_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 73,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = -10.0,
        .maxVal = 50.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = DewPointAverage_GraphHistory,
        .length  = sizeof(DewPointAverage_GraphHistory)
    }; hkGraphInit(&DewPointAverage_Graph);


    static u8 AbsHumidAverage_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t AbsHumidAverage_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 10,        // Start Y point
        .height = 50,   // End Y point  
        .minVal = 0.0,
        .maxVal = 100.0,
        .colour = 15,
        .borderColour = 15,
        .legendColour = 4,
        .cursorX = 0,
        .history = AbsHumidAverage_GraphHistory,
        .length  = sizeof(AbsHumidAverage_GraphHistory)
    }; hkGraphInit(&AbsHumidAverage_Graph);

    static u8 HumidAverage_GraphHistory[MAX_GRAPH_WIDTH];
    GraphConfig_t HumidAverage_Graph = {
        .x = 5,         // Start X point
        .width = 118,   // End X point
        .y = 73,        // Start Y point
        .height = 50,   // End Y point   
        .minVal = 0,
        .maxVal = 100.0,
        .colour = 10,
        .borderColour = 10,
        .legendColour = 4,
        .cursorX = 0,
        .history = HumidAverage_GraphHistory,
        .length  = sizeof(HumidAverage_GraphHistory)
    }; hkGraphInit(&HumidAverage_Graph);


    hkScreenSaverInit();
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

        if(screenSaver == true) hkScreenSaver(SSD1327_WIDTH, SSD1327_HEIGHT, 0);
        if((KY40_Position(0, hkKY40_GET) != lastPosition)) {
            screenSaver = false;
            resetScreeSaverTimer();
        }

        if(dataFromCore0 || (KY40_Position(0, hkKY40_GET) != lastPosition)) {
            lastPosition = KY40_Position(0, hkKY40_GET);

            hkGraphAddDataPoint(&SGP30_TVOC_Graph, (f32)data.sgp30.tvoc);
            hkGraphAddDataPoint(&SGP30_ECO2_Graph, (f32)data.sgp30.eco2);

            hkGraphAddDataPoint(&PMS5003_2_5_Graph, (f32)data.pms5003.pm2_5);
            hkGraphAddDataPoint(&PMS5003_10_Graph, (f32)data.pms5003.pm10);
            
            hkGraphAddDataPoint(&DS18B20_Graph, data.ds18b20.temperature);

            hkGraphAddDataPoint(&BME280_Temp_Graph, data.bme280.temperature);
            hkGraphAddDataPoint(&BME280_DewPoint_Graph, data.bme280.dewPoint);
            hkGraphAddDataPoint(&BME280_RelativeHumidity_Graph, data.bme280.humidity);
            hkGraphAddDataPoint(&BME280_AbsoluteHumidity_Graph, data.bme280.absoluteHumidity);
            
            hkGraphAddDataPoint(&DHT11_Temp_Graph, data.dht.temperature);
            hkGraphAddDataPoint(&DHT11_DewPoint_Graph, data.dht.dewPoint);
            hkGraphAddDataPoint(&DHT11_RelativeHumidity_Graph, data.dht.humidity);
            hkGraphAddDataPoint(&DHT11_AbsoluteHumidity_Graph, data.dht.absoluteHumidity);

            hkGraphAddDataPoint(&TempAverage_Graph, data.temperature);
            hkGraphAddDataPoint(&DewPointAverage_Graph, data.dewPoint);

            hkGraphAddDataPoint(&HumidAverage_Graph, data.humidity);
            hkGraphAddDataPoint(&AbsHumidAverage_Graph, data.absHumidity);
            
            // I still want to calculate data points, I just don't want to show them.
            if(screenSaver == true) continue;   

            switch(KY40_Position(0, hkKY40_GET)) {
                case 0:
                    hkClearBuffer();
                
                    hkGraphDrawAxes(&SGP30_TVOC_Graph);
                    hkGraphDrawLegend(&SGP30_TVOC_Graph, "TVOC (ppb)");

                    hkGraphDrawAxes(&SGP30_ECO2_Graph);
                    hkGraphDrawLegend(&SGP30_ECO2_Graph, "eCO2 (ppm)");

                    hkGraphDrawValue(&SGP30_TVOC_Graph, data.sgp30.tvoc, 0);
                    hkGraphDrawValueFollow(&SGP30_TVOC_Graph, data.sgp30.tvoc, 0);

                    hkGraphDrawValue(&SGP30_ECO2_Graph, data.sgp30.eco2, 0);
                    hkGraphDrawValueFollow(&SGP30_ECO2_Graph, data.sgp30.eco2, 0);

                    hkGraphDraw(&SGP30_TVOC_Graph);
                    hkGraphDraw(&SGP30_ECO2_Graph);
                    break;
                case 1:
                    hkClearBuffer();

                    hkGraphDrawAxes(&PMS5003_2_5_Graph);
                    hkGraphDrawAxes(&PMS5003_10_Graph);

                    hkGraphDrawLegend(&PMS5003_2_5_Graph, "PM2.5 (ug/m3)");
                    hkGraphDrawLegend(&PMS5003_10_Graph, "PM10 (ug/m3)");

                    hkGraphDrawValue(&PMS5003_2_5_Graph, data.pms5003.pm2_5, 0);
                    hkGraphDrawValueFollow(&PMS5003_2_5_Graph, data.pms5003.pm2_5, 0);

                    hkGraphDrawValue(&PMS5003_10_Graph, data.pms5003.pm10, 0);
                    hkGraphDrawValueFollow(&PMS5003_10_Graph, data.pms5003.pm10, 0);

                    hkGraphDraw(&PMS5003_2_5_Graph);
                    hkGraphDraw(&PMS5003_10_Graph);
                    break;
                case 2:
                    hkClearBuffer();

                    hkGraphDrawAxes(&DS18B20_Graph);
                    hkGraphDrawLegend(&DS18B20_Graph, "TEMP (C)");
                    hkGraphDrawValue(&DS18B20_Graph, data.ds18b20.temperature, 1);
                    hkGraphDrawValueFollow(&DS18B20_Graph, data.ds18b20.temperature, 1);

                    hkGraphDraw(&DS18B20_Graph);
                    break;
                case 3:
                    hkClearBuffer();

                    hkGraphDrawAxes(&BME280_Temp_Graph);
                    hkGraphDrawAxes(&BME280_DewPoint_Graph);

                    hkGraphDrawLegend(&BME280_Temp_Graph, "TEMP (C)");
                    hkGraphDrawLegend(&BME280_DewPoint_Graph, "DEW POINT (C)");

                    hkGraphDrawValue(&BME280_Temp_Graph, data.bme280.temperature, 1);
                    hkGraphDrawValueFollow(&BME280_Temp_Graph, data.bme280.temperature, 1);
                    
                    hkGraphDrawValue(&BME280_DewPoint_Graph, data.bme280.dewPoint, 1);
                    hkGraphDrawValueFollow(&BME280_DewPoint_Graph, data.bme280.dewPoint, 1);

                    hkGraphDraw(&BME280_Temp_Graph);
                    hkGraphDraw(&BME280_DewPoint_Graph);
                    break;
                case 4:
                    hkClearBuffer();

                    hkGraphDrawAxes(&DHT11_Temp_Graph);
                    hkGraphDrawAxes(&DHT11_DewPoint_Graph);

                    hkGraphDrawLegend(&DHT11_Temp_Graph, "TEMP (C)");
                    hkGraphDrawLegend(&DHT11_DewPoint_Graph, "DEW POINT (C)");

                    hkGraphDrawValue(&DHT11_Temp_Graph, data.dht.temperature, 1);
                    hkGraphDrawValueFollow(&DHT11_Temp_Graph, data.dht.temperature, 1);
                    
                    hkGraphDrawValue(&DHT11_DewPoint_Graph, data.dht.dewPoint, 1);
                    hkGraphDrawValueFollow(&DHT11_DewPoint_Graph, data.dht.dewPoint, 1);

                    hkGraphDraw(&DHT11_Temp_Graph);
                    hkGraphDraw(&DHT11_DewPoint_Graph);
                    break;
                case 5:
                    hkClearBuffer();

                    hkGraphDrawAxes(&BME280_RelativeHumidity_Graph);
                    hkGraphDrawAxes(&BME280_AbsoluteHumidity_Graph);

                    hkGraphDrawLegend(&BME280_RelativeHumidity_Graph, "REL HUMIDITY(%)");
                    hkGraphDrawLegend(&BME280_AbsoluteHumidity_Graph, "ABS HUMIDITY(%)");

                    hkGraphDrawValue(&BME280_RelativeHumidity_Graph, data.bme280.humidity, 1);
                    hkGraphDrawValueFollow(&BME280_RelativeHumidity_Graph, data.bme280.humidity, 1);
                    
                    hkGraphDrawValue(&BME280_AbsoluteHumidity_Graph, data.bme280.absoluteHumidity, 1);
                    hkGraphDrawValueFollow(&BME280_AbsoluteHumidity_Graph, data.bme280.absoluteHumidity, 1);

                    hkGraphDraw(&BME280_RelativeHumidity_Graph);
                    hkGraphDraw(&BME280_AbsoluteHumidity_Graph);
                    break;
                case 6:
                    hkClearBuffer();

                    hkGraphDrawAxes(&DHT11_RelativeHumidity_Graph);
                    hkGraphDrawAxes(&DHT11_AbsoluteHumidity_Graph);

                    hkGraphDrawLegend(&DHT11_RelativeHumidity_Graph, "REL HUMIDITY(%)");
                    hkGraphDrawLegend(&DHT11_AbsoluteHumidity_Graph, "ABS HUMIDITY(%)");

                    hkGraphDrawValue(&DHT11_RelativeHumidity_Graph, data.dht.humidity, 1);
                    hkGraphDrawValueFollow(&DHT11_RelativeHumidity_Graph, data.dht.humidity, 1);
                    
                    hkGraphDrawValue(&DHT11_AbsoluteHumidity_Graph, data.dht.absoluteHumidity, 1);
                    hkGraphDrawValueFollow(&DHT11_AbsoluteHumidity_Graph, data.dht.absoluteHumidity, 1);

                    hkGraphDraw(&DHT11_RelativeHumidity_Graph);
                    hkGraphDraw(&DHT11_AbsoluteHumidity_Graph);
                    break;
                case 7:
                    hkClearBuffer();

                    hkGraphDrawAxes(&AbsHumidAverage_Graph);
                    hkGraphDrawAxes(&HumidAverage_Graph);

                    hkGraphDrawLegend(&AbsHumidAverage_Graph, "AVG AH(%)");
                    hkGraphDrawLegend(&HumidAverage_Graph, "AVG RH(%)");

                    hkGraphDrawValue(&AbsHumidAverage_Graph, data.absHumidity, 1);
                    hkGraphDrawValueFollow(&AbsHumidAverage_Graph, data.absHumidity, 1);
                    
                    hkGraphDrawValue(&HumidAverage_Graph, data.humidity, 1);
                    hkGraphDrawValueFollow(&HumidAverage_Graph, data.humidity, 1);

                    hkGraphDraw(&AbsHumidAverage_Graph);
                    hkGraphDraw(&HumidAverage_Graph);
                    break;
                case 8:
                    hkClearBuffer();

                    hkGraphDrawAxes(&TempAverage_Graph);
                    hkGraphDrawAxes(&DewPointAverage_Graph);

                    hkGraphDrawLegend(&TempAverage_Graph, "AVG TEMP(C)");
                    hkGraphDrawLegend(&DewPointAverage_Graph, "AVG DEW PT(C)");

                    hkGraphDrawValue(&TempAverage_Graph, data.temperature, 1);
                    hkGraphDrawValueFollow(&TempAverage_Graph, data.temperature, 1);
                    
                    hkGraphDrawValue(&DewPointAverage_Graph, data.dewPoint, 1);
                    hkGraphDrawValueFollow(&DewPointAverage_Graph, data.dewPoint, 1);

                    hkGraphDraw(&TempAverage_Graph);
                    hkGraphDraw(&DewPointAverage_Graph);
                    break;
                default: 
                    hkClearBuffer();
                    hkDrawFastString(10, 50, "no view available");
                    break;
            }
        }

        hkDisplay();
        if(screenSaver == false) sleep_ms(250);
    }
}