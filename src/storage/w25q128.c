#include <config/arm.h>
#if hkFLASH_W25Q128

#include <stdio.h>
#include <string.h>

#include <hardware/spi.h>

#include <storage/storage.h>
#include <storage/flash.h>
#include <comm/spi.h>


b8 Flash_Write(const void* config, const void* packet) {return true;}
b8 Flash_Read(const void* config, void* packet) {return true;}

#endif