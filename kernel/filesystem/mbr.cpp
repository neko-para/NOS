#include "mbr.h"
#include "../io/disk.h"
#include "../util/string.h"

bool MBREntry::isValid() {
    uint8_t buf[sizeof (MBREntry)];
    memset(buf, 0, sizeof (buf));
    return memcmp(buf, this, sizeof (buf));
}

void MBR::load(uint32_t drive) {
    Disk::command(drive, 1, 0, Disk::C_READ);
    Disk::read(reinterpret_cast<uint16_t *>(this));
}