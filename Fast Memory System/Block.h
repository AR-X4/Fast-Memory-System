#ifndef BLOCK_H
#define BLOCK_H

#include "Types.h"

enum class Block : uint8_t
{
	Free = 0xC5, // Do not change values
	Used = 0xA3  // Do not change values
};

#endif