#pragma once

#include "Core/CoreTypes.h"

#define JOB_SYSTEM_NULL_HANDLE 0
#define JOB_SYSTEM_JOB_ENTRY_POINT(func) ((void(*)(void*))func)

enum
{
    JOB_SYSTEM_MAX_NUM_WORKER_THREADS = 128,
    JOB_SYSTEM_MAX_NUM_FIBERS = 256,
    JOB_SYSTEM_MAX_NUM_JOBS = 4096,
};

namespace HE
{

using JobSystemAtomicCounterHandle = uint32;

using JobSystemJobFunc = void(*)(void*);

struct JobSystemJobDecl
{
    JobSystemJobFunc jobFunc;
    void* data;
};

extern void JobSystemInit(uint32 numWorkerThreads, uint32 numFibers, uint32 fiberStackSize);
extern void JobSystemExit();
extern JobSystemAtomicCounterHandle JobSystemRunJobs(JobSystemJobDecl* jobDecls, uint32 numJobs);
extern void JobSystemWaitForCounter(JobSystemAtomicCounterHandle counterHandle, uint32 condition);
extern void JobSystemWaitForCounterAndFree(JobSystemAtomicCounterHandle counterHandle, uint32 condition);
extern void JobSystemWaitForCounterAndFreeWithoutFiber(JobSystemAtomicCounterHandle counterHandle);

}