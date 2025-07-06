#pragma once
#include <defines.h>
#include <string.h>

#include <core/logger.h>

#include <pico/unique_id.h>
#include <lwip/ip_addr.h>
#include <lwip/apps/mqtt.h>
#include <lwip/apps/mqtt_priv.h>
#include <lwip/dns.h>
#include <lwip/altcp_tls.h>

#define BIT_MQTT_CONNECTED (1 << 0)

#ifndef MQTT_QOS0
#define MQTT_QOS0  0
#endif

#ifndef MQTT_QOS1
#define MQTT_QOS1  1
#endif

#ifndef MQTT_QOS2
#define MQTT_QOS2  2
#endif


typedef struct MQTT_Config_t {
    struct mqtt_connect_client_info_t clientInfo;
    ip_addr_t      mqttServerAddress;
    u16            mqttServerPort;
    mqtt_client_t* mqttInstance;
    char*          topic;
    u32            len;
    b8             connected;
    u32            subscribeCount;
    b8             disconnected;
} MQTT_Config_t;


static void hkUniqueID(const char* deviceName, char* clientID, size_t size) {
    char uniqueID[9];
    pico_get_unique_board_id_string(uniqueID, sizeof(uniqueID));
    for(size_t i = 0; uniqueID[i]; ++i) uniqueID[i] = (char)tolower((unsigned char)uniqueID[i]);

    size_t nameLen = strlen(deviceName);
    size_t idLen   = strlen(uniqueID);
    if((nameLen + idLen + 1) > size) {
        HWARN("hkUniqueID(): Out buffer is too small. Truncating output to %d characters", size);
        idLen = (size - nameLen - 1);
    }

    memcpy(clientID, deviceName, nameLen);
    memcpy(clientID + nameLen, uniqueID, idLen);
    clientID[nameLen + idLen] = '\0';
}


i32 MQTT_Init(MQTT_Config_t* config);