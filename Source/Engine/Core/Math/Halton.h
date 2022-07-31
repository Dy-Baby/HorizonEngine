#pragma once

#include "Core/CoreTypes.h"

namespace HE
{
inline float Halton(int32 index, int32 base)
{
	float result = 0.0f;
	float invBase = 1.0f / base;
	float fraction = invBase;
	while (index > 0)
	{
		result += (index % base) * fraction;
		index /= base;
		fraction *= invBase;
	}
	return result;
}
}