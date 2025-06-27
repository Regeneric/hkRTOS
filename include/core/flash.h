#pragma once
#include <FreeRTOS.h>
#include <task.h>

#include <defines.h>
#include <pico/sync.h>
#include <hardware/flash.h>


enum {
    FLASH_HEADER = 0xA1A2A3A4u,
};

#ifndef FLASH_SECTOR_SIZE
    #define FLASH_SECTOR_SIZE 4096
#endif
#define STORAGE_OFFSET   (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define STORAGE_ADDR     (XIP_BASE + STORAGE_OFFSET)

typedef struct {
    u32 magic;
    u32 checksum;
    u8  payload[];
} __attribute__((packed)) FlashConfig_t;

static u32 hkFlashCRC(const uint8_t* data, size_t len) {
    uint32_t sum = 0;
    for(size_t i = 0; i < len; ++i) sum += data[i];
    return sum;
}


void hkFlashReadRaw(u8* data, size_t len);
static void __not_in_flash_func(hkFlashWriteRaw)(const FlashConfig_t* data, size_t size) {
    u32 irq_state = save_and_disable_interrupts();

    vTaskSuspendAll();
    flash_range_erase(STORAGE_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(STORAGE_OFFSET, (const u8*)data, size);
    xTaskResumeAll();
    restore_interrupts(irq_state);
}

b8 hkFlashRead(void* data, size_t size);
void hkFlashWrite(const void* data, size_t size);