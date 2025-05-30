#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>

#include <comm/i2c.h>

void I2C_Init(const I2C_Config_t* config) {
    i2c_init(config->i2c, config->speed);

    gpio_set_function(config->sda, GPIO_FUNC_I2C);
    gpio_set_function(config->scl, GPIO_FUNC_I2C);

    gpio_pull_up(config->sda);
    gpio_pull_up(config->scl);
}

void I2C_Stop(const I2C_Config_t* config) {
    gpio_deinit(config->sda);
    gpio_deinit(config->scl);

    i2c_deinit(config->i2c);
}