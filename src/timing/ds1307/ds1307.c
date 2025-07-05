#include <time.h>

#include <core/logger.h>
#include <core/bcd.h>
#include <timing/ds1307/ds1307.h>


i32 DS1307_Init(I2C_Config_t* i2c, DS1307_Config_t* config, DS1307_DataPacket_t* dt) {
    HTRACE("ds1307.c -> DS1307_Init(I2C_Config_t*, DS1307_Config_t*, DS1307_DataPacket_t*):i32");
    HINFO("Initializing DS1307 RTC module...");

    mutex_enter_blocking(i2c->mutex);
    HTRACE("DS1307_Init(): I2C mutex acquired");

    u8 commands[2];
    commands[0] = DS1307_REG_SECONDS;
    i32 status = i2c_write_blocking(i2c->i2c, config->address, commands, 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("DS1307_Init(): Couldn't write data to device at address: 0x%x", config->address);
        mutex_exit(i2c->mutex);

        HTRACE("DS1307_Init(): I2C mutex released");
        
        return status;
    }

    status = i2c_read_blocking(i2c->i2c, config->address, (commands + 1), 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("DS1307_Init(): Could not start sensor data read");
        mutex_exit(i2c->mutex);

        HTRACE("DS1307_Init(): I2C mutex released");
        
        return status;
    }


    b8 clockHalted = (commands[1] & 0x80) != 0;
    if(clockHalted) {
        if(!dt) {
            HERROR("DS1307_Init(): You must set current date and time first!");
            mutex_exit(i2c->mutex);
            return false;
        }
    } else {
        if(config->reset == false) {
            HINFO("DS1307 RTC module already initialized!");
            mutex_exit(i2c->mutex);
            return true;
        }
    }


    commands[1] &= 0x7F;
    status = i2c_write_blocking(i2c->i2c, config->address, commands, sizeof(commands), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("DS1307_Init(): Couldn't write data to device at address: 0x%x", config->address);
        mutex_exit(i2c->mutex);

        HTRACE("DS1307_Init(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("SGP30_Read(): I2C mutex released");


    // YYYY-MM-DDThh:mm:ss - ISO datetime
    HINFO("Setting current date and time to: %04d-%02d-%02dT%02d:%02d:%02d", dt->year, dt->month, dt->day, dt->hour, dt->min, dt->sec);

    u8 tbuf[8] = {
        DS1307_REG_SECONDS,
        intToBCD(dt->sec),
        intToBCD(dt->min),
        intToBCD(dt->hour),
        intToBCD(dt->dayName),
        intToBCD(dt->day),
        intToBCD(dt->month),
        intToBCD(dt->year % 100)
    };

    mutex_enter_blocking(i2c->mutex);
    HTRACE("DS1307_Init(): I2C mutex acquired");

    status = i2c_write_blocking(i2c->i2c, config->address, tbuf, sizeof(tbuf), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("DS1307_Init(): Couldn't write data to device at address: 0x%x", config->address);
        mutex_exit(i2c->mutex);

        HTRACE("DS1307_Init(): I2C mutex released");
        HERROR("DS1307 RTC module could not be initalized.");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("SGP30_Read(): I2C mutex released");

    HINFO("DS1307 RTC module has been initalized.");
    return true;
}   



u32 DS1307_ReadDateTime(I2C_Config_t* i2c, DS1307_Config_t* config, DS1307_DataPacket_t* dt) {
    HTRACE("ds1307.c -> DS1307_ReadDateTime(I2C_Config_t*, DS1307_Config_t*, DS1307_DataPacket_t*):i32");

    u8 tbuf[7];

    mutex_enter_blocking(i2c->mutex);
    HTRACE("DS1307_Init(): I2C mutex acquired");

    i32 status = i2c_write_blocking(i2c->i2c, config->address, DS1307_REG_SECONDS, 1, true);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("DS1307_Init(): Couldn't write data to device at address: 0x%x", config->address);
        mutex_exit(i2c->mutex);

        HTRACE("DS1307_Init(): I2C mutex released");
        HERROR("DS1307 RTC module could not be initalized.");
        
        return status;
    }

    status = i2c_read_blocking(i2c->i2c, config->address, tbuf, sizeof(tbuf), false);
    if(status == PICO_ERROR_GENERIC) {
        HDEBUG("DS1307_Init(): Could not start sensor data read");
        mutex_exit(i2c->mutex);

        HTRACE("DS1307_Init(): I2C mutex released");
        
        return status;
    }

    mutex_exit(i2c->mutex);
    HTRACE("SGP30_Read(): I2C mutex released");

    dt->sec     = BCDToInt(tbuf[0]);
    dt->min     = BCDToInt(tbuf[1]);
    dt->hour    = BCDToInt(tbuf[2]);
    dt->dayName = BCDToInt(tbuf[3]);
    dt->day     = BCDToInt(tbuf[4]);
    dt->month   = BCDToInt(tbuf[5]);
    dt->year    = BCDToInt(tbuf[6] + 2000);

    struct tm timeinfo = {
        .tm_year  = (dt->year  - 1900),
        .tm_mon   = (dt->month - 1),
        .tm_mday  =  dt->day,
        .tm_hour  =  dt->hour,
        .tm_min   =  dt->min,
        .tm_sec   =  dt->sec,
        .tm_isdst = -1  // Auto detect DST
    };

    time_t localTimestamp = mktime(&timeinfo);
    // dt->timestamp = (u32)(localTimestamp - (config->utcOffset * 3600));
    u32 utcTimestamp = (u32)(localTimestamp - (config->utcOffset * 3600));
    return utcTimestamp;
}

void DS1307_TimeStamp(u32 timestamp, char* buffer, size_t len) {
    HTRACE("ds1307.c -> DS1307_TimeStamp(DS1307_DataPacket_t*, char*, size_t):void");

    struct tm* timeinfo;
    timeinfo = gmtime((long long int*)(timestamp));

    strftime(buffer, len, "%Y-%m-%dT%H:%M:%S", timeinfo);
}