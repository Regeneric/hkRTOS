#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <event_groups.h>

#include <pico/cyw43_arch.h>
#include <lwip/netif.h>
#include <lwip/ip4_addr.h>
#include <lwip/dns.h>
#include <lwip/def.h> 
#include <lwip/apps/httpd.h>
#include <lwip/apps/fs.h>

#include <hardware/watchdog.h>

#include <core/logger.h>
#include <core/flash.h> // TODO: it should be storage/flash
#include <storage/storage.h>
#include <storage/eeprom.h>

#include <comm/i2c.h>
#include <comm/network/wifi.h>
#include <comm/network/ping.h>


static WiFi_TaskParams_t* sgParams;

static const char* configHandler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]);
static u16_t ssiHandler(int iIndex, int* pcInsert, int iInsertLen);

static const char* ssiTags[] = {"ssid", "passwd"};
static const tCGI cgiHandlers[] = {
    {
        "/config.cgi", configHandler
    }
};


extern EventGroupHandle_t xSystemStateEventGroup;
void vWifiManagerTask(void* pvParameters) {
    HTRACE("wifi.c -> RTOS:vWifiManagerTask(void*):void");

    WiFi_TaskParams_t* params = (WiFi_TaskParams_t*)pvParameters;
    sgParams = params;

    WiFiCredentials_t creds = {0};
    strncpy(creds.ssid, params->wifi->ssid, SSID_MAXLEN - 1);
    creds.ssid[SSID_MAXLEN - 1] = '\0';

    strncpy(creds.pass, params->wifi->password, PASS_MAXLEN - 1);
    creds.pass[PASS_MAXLEN - 1] = '\0';

    UBaseType_t coreID = portGET_CORE_ID();
    
    if(cyw43_arch_init_with_country(params->wifi->country) != 0) {
        HFATAL("vWifiManagerTask(): Failed to initialize WiFi chip.");
        vTaskSuspend(NULL);     // Suspend task indefinitely on fatal error
    } 
    
    HINFO("WiFi chip has been initialized.");
    cyw43_arch_enable_sta_mode();

    // Check if there are credentials stored in the EEPROM Memory
    if(EEPROM_ReadBlob(params->i2c, 0x00, (u8*)&creds, sizeof(creds))) HINFO("Valid saved credentials has been found, skipping config values (SSID='%s', PASS='%s')", creds.ssid, creds.pass);
    else HINFO("No saved credentials has been found, using config values (SSID='%s', PASS='%s')", creds.ssid, creds.pass);

    // Initial wifi connection attempt
    if(cyw43_arch_wifi_connect_timeout_ms(creds.ssid, creds.pass, params->wifi->authType, 30000)) {
        HERROR("vWifiManagerTask(): Failed to connect to WiFi after 30s timeout.");
        cyw43_arch_disable_sta_mode();
        xEventGroupClearBits(xSystemStateEventGroup, WIFI_CONNECTED_BIT);
    } else {
        HINFO("WiFi connected successfully!");

        // Register ICMP hearbeat
        params->wifi->ipGateway = netif_ip4_gw(netif_default);
        initHeartbeat(params->wifi->ipGateway);

        xEventGroupSetBits(xSystemStateEventGroup, WIFI_CONNECTED_BIT);
    }

    while(FOREVER) {
        HTRACE("vWifiManagerTask(): Running on core {%d}", (u16)coreID);
        
        EventBits_t systemState = xEventGroupGetBits(xSystemStateEventGroup);
        if(systemState & BIT_MODE_CONFIG) {
            HDEBUG("vWifiManagerTask(): Starting in AP mode");

            ip4_addr_t gw, mask, ip;
            IP4_ADDR(&ip,   192, 168, 250, 1);
            IP4_ADDR(&gw,   192, 168, 250, 1);
            IP4_ADDR(&mask, 255, 255, 255, 0);

            cyw43_arch_enable_ap_mode("Pico-Indoor-Setup", NULL, CYW43_AUTH_OPEN);
            HINFO("Access Point 'Pico-Indoor-Setup' has been initialized.");

            struct netif *netif = &cyw43_state.netif[CYW43_ITF_AP];
            netif_set_addr(netif, &ip, &mask, &gw);

            httpd_init();
            http_set_cgi_handlers(cgiHandlers, 1);
            // http_set_ssi_handler(ssiHandler, ssiTags, 1);
            HINFO("HTTP server has been initialized on address http://%s", ip4addr_ntoa(&ip));

            // Config mode loop - blink LED to indicate we're not operating normally
            while(xEventGroupGetBits(xSystemStateEventGroup) & BIT_MODE_CONFIG) {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(250));

                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(250));
            }

            HINFO("Exiting CONFIGURATION mode. Rebooting to apply settings...");
            cyw43_arch_disable_ap_mode();

            vTaskDelay(pdMS_TO_TICKS(1000));
            cyw43_arch_deinit();
            
            watchdog_reboot(0, 0, 0);
        } else {
            u32 linkStatus = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
            if(linkStatus == CYW43_LINK_JOIN) {   // TODO: it won't work as intended
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                xEventGroupSetBits(xSystemStateEventGroup, WIFI_CONNECTED_BIT);
                
                HDEBUG("vWifiManagerTask(): IP address: %s", ip4addr_ntoa(netif_ip4_addr(netif_default)));
                params->wifi->ipAddress = netif_ip4_addr(netif_default);
                params->wifi->ipMask    = netif_ip4_netmask(netif_default);
                params->wifi->ipGateway = netif_ip4_gw(netif_default);

                vTaskDelay(pdMS_TO_TICKS(30000));
            } else {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
                vTaskDelay(pdMS_TO_TICKS(250));

                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
                vTaskDelay(pdMS_TO_TICKS(250));

                xEventGroupClearBits(xSystemStateEventGroup, WIFI_CONNECTED_BIT);
                HERROR("vWifiManagerTask(): WiFi disconnected. Reason code: %d. Attempting to reconnect...", linkStatus);

                if(cyw43_arch_wifi_connect_timeout_ms(params->wifi->ssid, params->wifi->password, params->wifi->authType, 10000)) {
                    HERROR("vWifiManagerTask(): WiFi reconnect attempt failed.");
                } else {
                    HDEBUG("vWifiManagerTask(): WiFi reconnected successfully!");

                    // Register ICMP hearbeat
                    params->wifi->ipGateway = netif_ip4_gw(netif_default);
                    initHeartbeat(params->wifi->ipGateway);
                }
                
                vTaskDelay(pdMS_TO_TICKS(10000));
            }
        }
    }
}


static const char* configHandler(int iIndex, int iNumParams, char* pcParam[], char* pcValue[]) {
    HTRACE("wifi.c -> s:cgiHandler(int, int, char**, char**):const char*");
    HDEBUG("CGI handler called with %d parameters", iNumParams);

    WiFiCredentials_t creds = {0};
    for(size_t i = 0; i < iNumParams; i++) {
        if(strcmp(pcParam[i], "ssid") == 0) {
            HINFO("Received SSID: %s", pcValue[i]);
            strncpy(creds.ssid, pcValue[i], SSID_MAXLEN - 1);
        } else if(strcmp(pcParam[i], "pass") == 0) {
            HINFO("Received password: %s", pcValue[i]);
            strncpy(creds.pass, pcValue[i], PASS_MAXLEN - 1);
        }
    }

    // DataPacket_t eepromWrite = {
    //     .address = 0x00,
    //     .data    = (u8*)&creds,
    //     .size    = sizeof(creds)
    // };

    EEPROM_WriteBlob(sgParams->i2c, 0x00, (u8*)&creds, sizeof(creds));
    HINFO("WiFi credentials has been saved in flash memory (SSID='%s', PASS='%s')", creds.ssid, creds.pass);
    
    HINFO("Configuration received. System will apply settings.");
    xEventGroupClearBits(xSystemStateEventGroup, BIT_MODE_CONFIG);

    return "/index.shtml";
}