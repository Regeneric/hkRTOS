#include <hardware/gpio.h>
#include <input/encoders/ky40.h>
#include <core/logger.h>

typedef struct KY40_State_t {
    u8  clk;
    u8  dt;
    u8  btn;
    vi8 position;
    vi8 lastClk;
} KY40_State_t;

static KY40_State_t encoders[hkKY40_ENCODERS_COUNT];


void __not_in_flash_func(KY40_ISR)(uint gpio, u32 events) {
    KY40_State_t* encoder = NULL;
    for(u8 i = 0; i < hkKY40_ENCODERS_COUNT; ++i) {
        if(encoders[i].clk == gpio) {
            u8 clk = gpio_get(gpio);
            
            if(clk != encoders[i].lastClk) {
                encoders[i].position += (gpio_get(encoders[i].dt) != clk) ? +1 : -1;
            }

            encoders[i].lastClk = clk;
            break;
        } 
    }
}

void KY40_InitSingle(KY40_State_t* encoder) {
    gpio_init(encoder->clk);
    gpio_init(encoder->dt);
    gpio_init(encoder->btn);

    gpio_set_dir(encoder->clk, GPIO_IN);
    gpio_set_dir(encoder->dt , GPIO_IN);
    gpio_set_dir(encoder->btn, GPIO_IN);

    gpio_pull_up(encoder->clk);
    gpio_pull_up(encoder->dt);
    gpio_pull_up(encoder->btn);

    encoder->lastClk = gpio_get(encoder->clk);

    gpio_set_irq_enabled_with_callback(encoder->clk, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &KY40_ISR);
}

void KY40_InitAll(const KY40_Config_t* config) {
    for(u8 i = 0; i < hkKY40_ENCODERS_COUNT; ++i) {
        encoders[i].clk = config[i].clk;
        encoders[i].dt  = config[i].dt;
        encoders[i].btn = config[i].btn;

        encoders[i].position = 0;
        KY40_InitSingle(&encoders[i]);
    }
}

u8 KY40_Position(u8 index, i8 value) {
    if(encoders[index].position > 40) encoders[index].position = 0;
    if(encoders[index].position <  0) encoders[index].position = 40; 

    if(value < 0)  return encoders[index].position/2;
    if(value > 20) return PICO_ERROR_GENERIC;

    encoders[index].position = value*2;
    return encoders[index].position/2;
}