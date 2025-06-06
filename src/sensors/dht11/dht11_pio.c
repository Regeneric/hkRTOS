#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>

#include <hardware/pio.h>
#include <hardware/clocks.h>

#include <sensors/sensors.h>
#include <sensors/dht11.h>
#include <sensors/dht11.pio.h>

#if hkDHT_USE_SENSOR && (hkDHT_USE_PIO && !hkDHT_USE_DMA)
static u32 sgDHT11_SM_Offset = 0;

void DHT11_Init(DHT_Config_t* config) {
    if(config->status == DHT_INIT) printf("Reinitalizing DHT sensor...\n");

    sgDHT11_SM_Offset = pio_add_program(config->pio, &dht11_program);
    dht11_program_init(config->pio, config->sm, sgDHT11_SM_Offset, config->gpio);
    
    #if hkDHT11_USE_DMA

    #endif

    config->status = DHT_READ_SUCCESS;
    return;
}

b8 DHT11_Read(DHT_Config_t* config) {
    if(config->length < 5) return false;
    if(config->status == DHT_READ_IN_PROGRESS) return false;
    
    memset(config->data, 0, config->length);

    pio_sm_clear_fifos(config->pio, config->sm);
    pio_sm_set_enabled(config->pio , config->sm, true);
    pio_sm_put_blocking(config->pio, config->sm, 20000U);
    pio_sm_put_blocking(config->pio, config->sm, 30U);

    u32 val = 0;
    for(u8 byte = 0; byte < 5; ++byte) {
        u32 val = pio_sm_get_blocking(config->pio, config->sm);
        config->data[byte] = (u8)(val & 0xFF);
    } pio_sm_set_enabled(config->pio , config->sm, false);
    
    u8 checksum = (config->data[0] + config->data[1] + config->data[2] + config->data[3]) & 0xFF; 
    if(checksum != config->data[4]) {
        printf("Data read failed. Invalid checksum: 0x%x  0x%x\n", checksum, config->data[4]);
        return false;
    } return true;
}
#endif