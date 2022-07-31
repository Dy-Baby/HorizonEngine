#include "Guid.h"

namespace HE
{

#if defined(HE_PLATFORM_WINDOWS)
#include <objbase.h>
	void GenerateUuidImpl(Guid* uuid)
	{
		ASSERT(CoCreateGuid((GUID*)uuid) == S_OK);
	}
#endif

Guid Guid::Generate()
{
	Guid uuid;
	GenerateUuidImpl(&uuid);
	return uuid;
}

std::string Guid::ToString(const Guid& guid)
{
	return std::string();
}

}