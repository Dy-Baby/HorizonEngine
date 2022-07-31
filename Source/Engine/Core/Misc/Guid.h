#pragma once

#include "Core/CoreTypes.h"

namespace HE
{
/**
 * Implements a Universally Unique Identifier.
 */
struct Guid
{
	/** Generate a new Uuid. */
	static Guid Generate();
	/** Convert Uuid to std::string. */
	static std::string ToString(const Guid& uuid);
	/** The first component. */
	uint32 a;
	/** The second component. */
	uint32 b;
	/** The third component. */
	uint32 c;
	/** The fourth component. */
	uint32 d; 
	FORCEINLINE bool operator==(Guid& rhs) const noexcept
	{
		return (a == rhs.a) && (b == rhs.b) && (c == rhs.c) && (d == rhs.d);
	}
	FORCEINLINE bool operator!=(Guid& rhs) const noexcept
	{
		return !(*this == rhs);
	}
};

/** The Uuid should be 128 bits. */
static_assert(sizeof(Guid) == 16);
}