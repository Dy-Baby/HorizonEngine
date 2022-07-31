#pragma once

#include "Core/CoreCommon.h"

namespace HE
{
namespace Crc
{
	uint32 Crc32(const void* data, uint64 size, uint32 crc = 0);
};
}