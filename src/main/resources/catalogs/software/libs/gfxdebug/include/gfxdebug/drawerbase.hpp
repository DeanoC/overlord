//
// Created by deano on 8/21/22.
//
#pragma once

#include "core/core.h"

namespace GfxDebug {

struct DrawerBase {

	virtual void setBackgroundColour( uint8_t index ) = 0;

	virtual void setPenColour( uint8_t index ) = 0;

	virtual void Clear() const = 0;

	virtual void PutString( int col, int row, char const *str ) const = 0;

	virtual void PutChar( int col, int row, char c ) const = 0;

	virtual void SetPixel( int x, int y, uint8_t val ) = 0;

};

} // end namespace