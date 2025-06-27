#pragma once
#include <defines.h>
#include <comm/i2c.h>

#define WIFI_CONNECTED_BIT (1 << 0)

typedef struct WIFI_Config_t {
    const char* ssid;
    const char* password;
    u32         authType;
    u32         country;
    const ip4_addr_t* ipAddress;
    const ip4_addr_t* ipMask; 
    const ip4_addr_t* ipGateway;
} WIFI_Config_t;


#define SSID_MAXLEN  64
#define PASS_MAXLEN  64

typedef struct {
    char ssid[SSID_MAXLEN];
    char pass[PASS_MAXLEN];
} WiFiCredentials_t;


typedef struct {
    I2C_Config_t* i2c;
    WIFI_Config_t* wifi;
} WiFi_TaskParams_t;

u8 WIFI_Init(WIFI_Config_t* config);
u8 WIFI_Stop(WIFI_Config_t* config);

void vWifiManagerTask(void* pvParameters);