#pragma once

#include "Core/CoreCommon.h"

#include <chrono>

namespace HE
{
class Timestep
{
public:
	Timestep(float deltaTimeInSeconds = 0.0f) : deltaTimeInSeconds(deltaTimeInSeconds) {}
	float Seconds() const
	{
		return deltaTimeInSeconds;
	}
	float Milliseconds() const
	{
		return deltaTimeInSeconds * 1000.0f;
	}
private:
	float deltaTimeInSeconds;
};

class CpuTimer
{
public:

	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;

	static TimePoint getCurrentTimePoint()
	{
		return Clock::now();
	}

	CpuTimer() : startTime(Clock::now()) {}

	void Reset()
	{
		startTime = Clock::now();
	}

	float ElapsedSeconds() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - startTime).count() * 0.001f * 0.001f * 0.001f;
	}

	float ElapsedMilliseconds() const
	{
		return ElapsedSeconds() * 1000.0f;
	}

private:

	TimePoint startTime;
};

class Time
{
public:
	static void Reset()
	{
		Timer.Reset();
	}
	static float Now()
	{
		return Timer.ElapsedSeconds();
	}
	static float GetDeltaTime()
	{
		return DeltaTime;
	}
private:
	static CpuTimer Timer;
	static float DeltaTime;
	static uint32 FrameCounter;
};
}
