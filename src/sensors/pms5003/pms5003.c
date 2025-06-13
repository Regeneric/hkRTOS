#include <core/logger.h>
#include <sensors/pms5003/pms5003.h>


static b8 PMS5003_ValidatePacket(PMS5003_Config_t* config) {
    HTRACE("pms5003.c -> s:PMS5003_ValidatePacket(PMS5003_Config_t*):b8");

    if(config->rawData[0] != 0x42 || config->rawData[1] != 0x4D) {
        HWARN("PMS5003_ValidatePacket(): Validation failed; invalid start bytes.");
        return false;
    }

    u16 checksum = 0;
    for(u8 i = 0; i < (hkPMS_PACKET_LENGTH-2); i++) checksum += config->rawData[i];

    u16 sensorChecksum = (config->rawData[30] << 8) | config->rawData[31];
    return (checksum == sensorChecksum);
}


void PMS5003_ProcessData(PMS5003_Config_t* config, PMS5003_DataPacket_t* data) {
    HTRACE("pms5003.c -> PMS5003_ProcessData(PMS5003_Config_t*, PMS5003_DataPacket_t*):void");
    if(!PMS5003_ValidatePacket(config)) return;

    data->pm1   = (config->rawData[4] << 8) | config->rawData[5];
    data->pm2_5 = (config->rawData[6] << 8) | config->rawData[7];
    data->pm10  = (config->rawData[8] << 8) | config->rawData[9];
}

b8 PMS5003_Read(UART_Config_t* uart, PMS5003_Config_t* config) {
    HTRACE("pms5003.c -> PMS5003_Read(UART_Config_t*, PMS5003_Config_t*):b8");

    UART_ReadPacket(uart);
    config->rawData = uart->data;
    return true;
}