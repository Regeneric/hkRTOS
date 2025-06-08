#pragma once
#include <defines.h>

b8 Flash_Write(const void* config, const void* packet);
b8 Flash_Read(const void* config, void* packet);