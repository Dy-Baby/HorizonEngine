#pragma once

#include <HorizonEngine.h>

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
		LOG_ERROR("Failed to create application.");
	}
	while (TickApplication(app))
	{
		PluginSystem::HotReload();
	}
	DestroyApplication(app);
}

int main(int argc, char** argv)
{
	LogSystem::EnableLogging();
	JobSystem::Init(GetNumberOfProcessors(), JOB_SYSTEM_NUM_FIBIERS, JOB_SYSTEM_FIBER_STACK_SIZE);
	{
		RunApplicationJobData jobData = {
			.argc = argc,
			.argv = argv
		};
		JobSystem::JobDecl jobDecl = {
			JOB_SYSTEM_JOB_ENTRY_POINT(RunApplication),
			&jobData
		};
		JobSystemAtomicCounterHandle atomicCounter = JobSystem::RunJobs(&jobDecl, 1);
		JobSystem::WaitForCounterAndFreeWithoutFiber(atomicCounter);
	}
	JobSystem::Shutdown();
	return EXIT_SUCCESS;
}
