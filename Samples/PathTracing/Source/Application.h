namespace HE
{
#define APPLICATION_NAME "Path Tracing Example"
#define APPLICATION_VERSION 1

class Application;

extern Application* CreateApplication(int argc, char** argv);
extern void DestroyApplication(Application* app);
extern bool TickApplication(Application* app);
}