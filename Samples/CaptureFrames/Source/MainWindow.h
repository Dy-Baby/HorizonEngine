#pragma once

#include <HorizonEngine.h>

#include <GLFW/glfw3.h>

#include <Windows.h>

enum class WindowState
{
	Normal,
	Minimized,
	Maximized,
	Fullscreen,
};

enum class WindowFlags
{
	None = 0,
	Resizable = 1 << 0,
	Borderless = 1 << 1,
};
ENUM_CLASS_OPERATORS(WindowFlags);

struct WindowCreateInfo
{
	uint32 width;
	uint32 height;
	const char* title;
	WindowFlags flags;
};

class MainWindow
{
public:

	MainWindow(WindowCreateInfo* info);
	~MainWindow();

	uint32 GetWidth() const
	{
		return width;
	}
	uint32 GetHeight() const
	{
		return height;
	}
	WindowState GetState() const
	{
		return state;
	}

	HWND GetNativeHandle();
	void ProcessEvents();
	bool ShouldClose() const;

	GLFWwindow* handle;
private:
	uint32 width;
	uint32 height;
	const char* title;
	bool focused;
	WindowState state;
};

extern bool GLFWInit();
extern void GLFWExit();