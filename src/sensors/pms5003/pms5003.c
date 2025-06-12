#include <core/logger.h>
#include <sensors/pms5003/pms5003.h>


static b8 PMS5003_ValidatePacket(const UART_Config_t* config) {
    HTRACE("pms5003.c -> PMS5003_ValidatePacket(PMS5003_Config_t*):b8");

    if(config->data[0] != 0x42 || config->data[1] != 0x4D) {
        HWARN("PMS5003_ValidatePacket(): Validation failed; invalid start bytes.");
        return false;
    }

    u16 checksum = 0;
    for(u8 i = 0; i < (hkPMS_PACKET_LENGTH-2); i++) checksum += config->data[i];

    u16 sensorChecksum = (config->data[30] << 8) | config->data[31];
    return (checksum == sensorChecksum);
}


void PMS5003_ProcessData(UART_Config_t* uart, PMS5003_Config_t* config) {
    HTRACE("pms5003.c -> PMS5003_ProcessData(UART_Config_t*, PMS5003_Config_t*):void");
    if(!PMS5003_ValidatePacket(uart)) return;

    config->pm1   = (uart->data[4] << 8) | uart->data[5];
    config->pm2_5 = (uart->data[6] << 8) | uart->data[7];
    config->pm10  = (uart->data[8] << 8) | uart->data[9];
}

b8 PMS5003_Read(UART_Config_t* uart, PMS5003_Config_t* config) {
    HTRACE("pms5003.c -> PMS5003_Read(UART_Config_t*, PMS5003_Config_t*):b8");

    UART_ReadPacket(uart);
    config->rawData = uart->data;
    return true;
}