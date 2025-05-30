#pragma once
#include <defines.h>


b8 EEPROM_Write(const void* config, const void* packet);
b8 EEPROM_Read(const void* config, void* packet);