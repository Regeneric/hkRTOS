#include <hardware/gpio.h>
#include <input/encoders/ky40.h>
#include <core/logger.h>

static vu8 lastClk = 0; 
static vi8 position = 0;

static KY40_Config_t* sgKY40_Config;

static void KY40_Rotation_ISR(uint gpio, u32 events) { 
    u8 currentClk = gpio_get(sgKY40_Config->clk);
    if(currentClk != lastClk) {
        if(gpio_get(sgKY40_Config->dt) != currentClk) position++;
        else position--;

        if(position > 40) position = 0;
        if(position < 0)  position = 40;

        sgKY40_Config->position = position/2;
    } lastClk = currentClk;
}

static void KY40_Button_ISR(uint gpio, u32 events) {return;}


void KY40_Init(KY40_Config_t* config) {
    sgKY40_Config = config;
    position = config->position;

    gpio_init(config->clk);
    gpio_init(config->dt);
    gpio_ini(config->button);

    gpio_set_dir(config->clk, GPIO_IN);
    gpio_set_dir(config->dt , GPIO_IN);
    gpio_set_dit(config->button, GPIO_IN);

    gpio_pull_up(config->clk);
    gpio_pull_up(config->dt);
    gpio_pull_up(config->button);

    gpio_set_irq_enabled_with_callback(config->clk, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &KY40_Rotation_ISR);
    gpio_set_irq_enabled_with_callback(config->button, GPIO_IRQ_EDGE_FALL, true, &KY40_Button_ISR);
}