#pragma once

#include "Core/CoreTypes.h"

namespace HE
{
template<class T>
class Singleton
{
private:
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>&) = delete;
protected:
    static T* Instance;
    Singleton(void)
    {
        ASSERT(!Instance && "Only one instance can exist for a singlton class");
        Instance = static_cast<T*>(this);
    }
    ~Singleton()
    {
        ASSERT(Instance && "No instance of this singleton has been initialized.");
        Instance = nullptr;
    }
public:
    static T& Get()
    {
        return *Instance;
    }
    static T* GetPtr()
    {
        return Instance;
    }
};
}
