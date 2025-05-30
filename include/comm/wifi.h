#pragma once
#include <defines.h>

typedef struct WIFI_Config_t {
    const char* ssid;
    const char* password;
    u32         authType;
    u32         country;
    ip4_addr_t* ipAddress;
    ip4_addr_t* ipMask; 
    ip4_addr_t* ipGateway;
} WIFI_Config_t;

u8 WIFI_Init(WIFI_Config_t* config);
u8 WIFI_Stop(WIFI_Config_t* config);