#pragma once
namespace DisplayPort::Display {

bool AuxRead(struct Connection *link, uint32_t address, uint32_t numBytes, uint8_t *data);
bool AuxWrite(struct Connection *link, uint32_t address, uint32_t byteCount, uint8_t const *data);

bool I2CRead(Connection *link, uint32_t address, uint32_t offset, uint32_t numBytes, uint8_t *data);

}