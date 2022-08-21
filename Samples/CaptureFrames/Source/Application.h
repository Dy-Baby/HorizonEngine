#pragma once

#include <HorizonEngine.h>

#include "CameraController.h"

#define HE_APPLICATION_NAME "Model Viewer"
#define HE_APPLICATION_VERSION HE_MAKE_VERSION(1, 0, 0)

class MainWindow;
class Application
{
public:

	static Application* Instance;

	static Application* GetInstance()
	{
		return Instance;
	}

	Application();
	~Application();

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	bool Init();
	void Exit();

	int Run();

	void Update(float deltaTime);
	void Render();
	void OnImGui();

	bool IsExitRequest() const
	{ 
		return isExitRequested;
	}

	MainWindow* GetMainWindow()
	{
		return window;
	}

private:

	HE::MemoryArena* arena;

	const char* name;

	bool isExitRequested;

	uint64 frameCounter; 

	MainWindow* window;

	HE::RenderScene* scene;

	HE::EntityHandle mainCamera;
	HE::EntityHandle sky;

	HE::RenderBackendSwapChainHandle swapChain;
	uint32 swapChainWidth;
	uint32 swapChainHeight;

	HE::ShaderCompiler* shaderCompiler;
	HE::RenderBackend* renderBackend;
	HE::UIRenderer* uiRenderer;

	HE::RenderContext* renderContext;

	SimpleFirstPersonCameraController cameraController;

	HE::SceneView* sceneView;
	HE::HybridRenderPipeline* renderPipeline;
};

extern int ApplicationMain();