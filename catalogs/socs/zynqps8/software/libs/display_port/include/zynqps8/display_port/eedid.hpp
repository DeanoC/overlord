#pragma once

namespace DisplayPort::Display::Eedid {

static const uint8_t EedidBlockSize = 128;

bool ReadBlock(struct Connection* display, uint32_t block, uint8_t* data);
void DumpBlock0(uint8_t const *data);

} // end namespace