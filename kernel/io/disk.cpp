#include "disk.h"
#include "io.h"
#include "../process/task.h"
#include "../util/new.h"

uint32_t Disk::currentDrive = 0xFFFF;
uint8_t Disk::presetCount = 1;

int32_t Disk::waitNoyBsy() {
    uint8_t status;
    int maxi = 10000;
    while (maxi--) {
        status = inb(P_STATUS);
        if (status & (F_ERR | F_DF)) {
            return E_ERROR;
        } else if ((status & F_BSY) == 0) {
            return E_OK;
        }
        Task::lockSchedule();
    }
    return E_TIMEOUT;
}

int32_t Disk::waitDrq() {
    uint8_t status;
    int maxi = 10000;
    while (maxi--) {
        status = inb(P_STATUS);
        if (status & (F_ERR | F_DF)) {
            return E_ERROR;
        } else if ((status & (F_BSY | F_DRQ)) == F_DRQ) {
            return E_OK;
        }
        Task::lockSchedule();
    }
    return E_TIMEOUT;
}

void Disk::command(uint32_t drive, uint8_t sectorCount, uint32_t lba, uint8_t cmd) {
    drive &= 1;
    outb(P_DEVICE, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));
    if (currentDrive != drive) {
        poll400();
        currentDrive = drive;
    }
    waitNoyBsy();
    outb(P_SECTOR_COUNT, sectorCount);
    presetCount = sectorCount;
    outb(P_LBA_LOW, lba & 0xFF);
    outb(P_LBA_MID, (lba >> 8) & 0xFF);
    outb(P_LBA_HIGH, (lba >> 16) & 0xFF);
    outb(P_COMMAND, cmd);
}

void Disk::read(uint16_t *data) {
    while (presetCount--) {
        waitDrq();
        for (int32_t i = 0; i < 256; i++) {
            *data++ = inw(P_DATA);
        }
        poll400();
    }
}

uint16_t *Disk::read() {
    uint16_t *buffer = new uint16_t[presetCount * 256];
    read(buffer);
    return buffer;
}

uint8_t Disk::poll400() {
    for (int i = 0; i < 15; i++) {
        inb(P_STATUS);;
    }
}
