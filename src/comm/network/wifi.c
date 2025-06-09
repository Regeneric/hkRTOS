#include <pico/cyw43_arch.h>
#include <lwip/netif.h>
#include <lwip/ip4_addr.h>

#include <core/logger.h>
#include <comm/network/wifi.h>

u8 WIFI_Init(WIFI_Config_t* config) {
    HTRACE("wifi.c -> WIFI_Init(WIFI_Config_t*):u8");

    u8 err = cyw43_arch_init_with_country(config->country);
    if(err != 0) {
        HDEBUG("WIFI_Init(): Wi-Fi init failed");
        return err;
    } cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    cyw43_arch_enable_sta_mode();

    err = cyw43_arch_wifi_connect_blocking(config->ssid, config->password, config->authType);
    if(err != 0) {
        HDEBUG("WIFI_Init(): Cannot connect to to %s network ; %d", config->ssid, err);
        return err;
    }

    struct netif* netif = netif_default;

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
        config->ipAddress = netif_ip4_addr(netif);
        config->ipMask = netif_ip4_netmask(netif);
        config->ipGateway = netif_ip4_gw(netif);
    #pragma GCC diagnostic pop 

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    return 0;
}

u8 WIFI_Stop(WIFI_Config_t* config) {
    HTRACE("wifi.c -> WIFI_Stop(WIFI_Config_t*):u8");

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    
    u8 err = cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    if(err != 0) return err;

    cyw43_arch_deinit();

    config->ipAddress = NULL;
    config->ipMask    = NULL;
    config->ipGateway = NULL;

    return 0;
}