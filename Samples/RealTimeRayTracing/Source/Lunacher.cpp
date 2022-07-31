#pragma once

#include "Application.h"

using namespace HE;

#define JOB_SYSTEM_NUM_FIBIERS 128
#define JOB_SYSTEM_FIBER_STACK_SIZE (JOB_SYSTEM_NUM_FIBIERS * 1024)

struct RunApplicationJobData
{
	int argc;
	char** argv;
};

static void RunApplication(RunApplicationJobData* data)
{
	Application* app = CreateApplication(data->argc, data->argv);
	if (!app)
	{
		HE_LOG_ERROR("Failed to create application.");
		return;
	}
	while (TickApplication(app))
	{
		// PluginSystem::HotReload();
	}
	DestroyApplication(app);
}

int main(int argc, char** argv)
{
	CoreGlobals::Init();
	LogSystem::EnableLogging();
	//JobSystem::Init(GetNumberOfProcessors(), JOB_SYSTEM_NUM_FIBIERS, JOB_SYSTEM_FIBER_STACK_SIZE);
	JobSystem::Init(1, JOB_SYSTEM_NUM_FIBIERS, JOB_SYSTEM_FIBER_STACK_SIZE);
	{
		RunApplicationJobData jobData = {
			.argc = argc,
			.argv = argv
		};
		JobDecl jobDecl = {
			JOB_SYSTEM_JOB_ENTRY_POINT(RunApplication),
			&jobData
		};
		JobSystemAtomicCounterHandle atomicCounter = JobSystem::RunJobs(&jobDecl, 1);
		JobSystem::WaitForCounterAndFreeWithoutFiber(atomicCounter);
	}
	JobSystem::Shutdown();
	CoreGlobals::Shutdown();
	return EXIT_SUCCESS;
}
