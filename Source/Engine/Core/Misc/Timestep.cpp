#include "Timestep.h"

namespace HE
{
CpuTimer Time::Timer = CpuTimer();
float Time::DeltaTime = 0.0f;
uint32 Time::FrameCounter = 0;
}
