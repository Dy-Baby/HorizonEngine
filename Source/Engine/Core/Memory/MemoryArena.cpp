#include "MemoryArena.h"

import HorizonEngine.Core.Math;

namespace HE
{

void* ArenaRealloc(MemoryArena* arena, void* ptr, uint64 oldSize, uint64 newSize, uint64 alignment, const char* file, uint32 line)
{
	ASSERT(arena);
	void* newPtr = nullptr;
	if (newSize)
	{
		newPtr = arena->Alloc(newSize, alignment);
		if (ptr)
		{
			memcpy(newPtr, ptr, Math::Min(oldSize, newSize));
			arena->Free(ptr, oldSize);
		}
	}
	else
	{
		arena->Free(ptr, oldSize);
	}
	return newPtr;
}

}