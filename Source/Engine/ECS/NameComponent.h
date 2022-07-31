#pragma once

#include "ECS/ECSCommon.h"

namespace HE
{

struct NameComponent
{
	std::pmr::string name;
	NameComponent() = default;
	NameComponent(const NameComponent& other) = default;
	NameComponent(const std::pmr::string& name) : name(name) {}
	inline operator std::pmr::string& () { return name; }
	inline operator const std::pmr::string& () const { return name; }
	inline void operator=(const std::pmr::string& str) { name = str; }
	inline bool operator==(const std::pmr::string& str) const { return name.compare(str) == 0; }
};

}
