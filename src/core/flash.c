#include <string.h>
#include <core/flash.h>

void flashReadRaw(u8* data, size_t len) {
    const u8* flashPtr = (const u8*)(XIP_BASE + STORAGE_OFFSET);
    memcpy(data, flashPtr, len);
}

static u8 buf[FLASH_SECTOR_SIZE] __attribute__((aligned(256))) = {0};
void hkFlashWrite(const void* data, size_t size) {
    size_t totalSize = sizeof(FlashConfig_t) + size;
    totalSize = (totalSize + 255) & ~255;

    FlashConfig_t* blk = (FlashConfig_t*)buf;

    blk->magic = FLASH_HEADER;
    memcpy(blk->payload, data, size);
    blk->checksum = hkFlashCRC(blk->payload, size);

    hkFlashWriteRaw(blk, size);
}

b8 hkFlashRead(void* data, size_t size) {
    const FlashConfig_t* blk = (const FlashConfig_t*)STORAGE_ADDR;

    if(blk->magic != FLASH_HEADER) return false;
    if(hkFlashCRC(blk->payload, size) != blk->checksum) return false;
    
    memcpy(data, blk->payload, size);
    return true;
}