#pragma once

#include <stdint.h>

class Disk {
public:
    enum {
        P_DATA = 0x1F0,
        P_ERROR = 0x1F1,
        P_FEATURE = 0x1F1,
        P_SECTOR_COUNT = 0x1F2,
        P_LBA_LOW = 0x1F3,
        P_LBA_MID = 0x1F4,
        P_LBA_HIGH = 0x1F5,
        P_DEVICE = 0x1F6,
        P_STATUS = 0x1F7,
        P_COMMAND = 0x1F7,

        F_ERR = 1 << 0,
        F_DRQ = 1 << 3,
        F_DF = 1 << 5,
        F_BSY = 1 << 7,

        C_READ = 0x20,
        C_WRITE = 0x30,

        E_OK = 0,
        E_ERROR = 1,
        E_TIMEOUT = 2
    };

    static int32_t waitNoyBsy();
    static int32_t waitDrq();
    static void command(uint32_t drive, uint8_t sectorCount, uint32_t lba, uint8_t cmd);
    static void read(uint16_t *data);
    static uint16_t *read();

private:
    static uint8_t poll400();

    static uint32_t currentDrive;
    static uint8_t presetCount;
};