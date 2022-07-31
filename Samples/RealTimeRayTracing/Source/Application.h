#pragma once

#include <HorizonEngine.h>
#include <HorizonEditor.h>

#define APPLICATION_NAME "Real-Time Ray Tracing"
#define APPLICATION_VERSION HE_MAKE_VERSION(1, 0, 0)

namespace HE
{

class Application;
extern Application* CreateApplication(int argc, char** argv);
extern void DestroyApplication(Application* app);
extern bool TickApplication(Application* app);

}