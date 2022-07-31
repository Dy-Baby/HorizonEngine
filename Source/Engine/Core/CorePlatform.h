#pragma once

#include "Core/CoreTypes.h"

namespace HE
{

struct DateTime
{
	uint64 time;
};

struct FileStatData
{
	bool isValid;
	bool isDirectory;
	bool isReadOnly;
	DateTime creationTime;
	DateTime accessTime;
	DateTime modificationTime;
	int64 size;
};

extern const char* GetBaseName(const char* path);
extern std::string GetDirectory(const std::string& path);
extern uint32 GetNumberOfProcessors();
extern FileStatData GetFileAttributeData(const char* path);
extern void SuspendCurrentThread(float seconds);
extern void YieldCPU();
extern uint32 GetCurrentThreadID();
extern void SwitchToAnotherFiber(uint64 handle);
extern uint64 OpenDLL(const char* path);
extern void* GetSymbolFromDLL(uint64 handle, const char* name);
extern void CloseDLL(uint64 handle);
extern bool GetExePath(char* path, uint32 size);
extern void FindFiles(const std::string& directory, const std::string& extension, std::vector<std::string>& outPaths);

}