#include "Application.h"

#include <HorizonEngine.h>
#include <HorizonEditor.h>

#include <VulkanRenderBackend.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace HE
{
struct MainWindow
{
	GLFWwindow* handle;
	uint32 width;
	uint32 height;
	String title;

	bool minimized;
	bool focused;

	RenderBackendSwapChainHandle swapChain;
	uint32 swapChainWidth;
	uint32 swapChainHeight;
};

class Application
{
public:
private:
	friend Application* CreateApplication(int argc, char** argv);
	friend void DestroyApplication(Application* app);
	friend bool TickApplication(Application* app);
	void InitMainWindow(); 
	void ShutdownMainWindow();
	bool InitRenderBackend();
	void ShutdownRenderBackend();
	void CreateResources();
	void DestroyResources();
	void UpdateGUI();
	void ResetCurrentSPP();
	void Render();

	MemoryAllocator* allocator;

	String name;
	uint32 version;
	bool exit;
	RenderBackend* renderBackend;
	uint32 numRenderDevices;
	Vector<uint32> renderDeviceMasks;
	uint32 mainRenderDeviceMask;

	SceneView view;
	MainWindow mainWindow;
	uint64 frameCounter;

	uint32 currentSPP;
	uint32 maxSPP;

	EditorCamera camera;
	EditorCameraController cameraController;
	EditorCameraControllerUserImpulseData cameraControllerUserImpulseData;

	RenderBackendShaderHandle pathTracingShader;
	RenderBackendShaderHandle finalShader;
	RenderBackendTextureHandle colorBuffer;
	RenderBackendTextureHandle finalBuffer;
};
Application* gApplication = nullptr;

void Application::InitMainWindow()
{
	if (glfwInit() != GLFW_TRUE)
	{
		LOG_ERROR("Failed to init glfw.");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mainWindow.width = 1280;
	mainWindow.height = 720;
	mainWindow.title = APPLICATION_NAME;

	mainWindow.handle = glfwCreateWindow(mainWindow.width, mainWindow.height, mainWindow.title.c_str(), nullptr, nullptr);
	if (mainWindow.handle == nullptr)
	{
		LOG_ERROR("Failed to create glfw window.");
		return;
	}

	int32 w, h;
	glfwGetWindowSize(gApplication->mainWindow.handle, &w, &h);
	mainWindow.width = w;
	mainWindow.height = h;

	glfwShowWindow(mainWindow.handle);
	glfwFocusWindow(mainWindow.handle);

	glfwSetWindowFocusCallback(mainWindow.handle, [](GLFWwindow * window, int focused)
	{
		gApplication->mainWindow.focused = (focused == GLFW_TRUE) ? true : false;
	});
	glfwSetWindowSizeCallback(mainWindow.handle, [](GLFWwindow* pGLFWwindow, int width, int height)
	{	
		glfwSetWindowSize(pGLFWwindow, width, height);
		int32 w, h;
		glfwGetWindowSize(pGLFWwindow, &w, &h);
		gApplication->mainWindow.width = w;
		gApplication->mainWindow.height = h;
		if (gApplication->mainWindow.width == 0 || gApplication->mainWindow.height == 0)
		{
			gApplication->mainWindow.minimized = true;
		}
	});

	HWND handle = glfwGetWin32Window(mainWindow.handle);

	mainWindow.swapChain = renderBackend->CreateSwapChain(mainRenderDeviceMask, (uint64)handle);
	mainWindow.swapChainWidth = mainWindow.width;
	mainWindow.swapChainHeight = mainWindow.height;
}

void Application::ShutdownMainWindow()
{
	if (mainWindow.handle)
	{
		glfwDestroyWindow(mainWindow.handle);
		glfwTerminate();
	}
}

bool Application::InitRenderBackend()
{
	renderBackend = vulkanRenderBackendAPI->CreateVulkanRenderBackend();
	if (!renderBackend->Init((uint32)(VulkanRenderBackendInitFlags::Surface | VulkanRenderBackendInitFlags::ValidationLayers)))
	{
		return false;
	}

	numRenderDevices = 1;
	Vector<PhysicalDeviceID> physicalDeviceIDs(numRenderDevices);
	physicalDeviceIDs[0] = 0;

	renderDeviceMasks.resize(numRenderDevices);
	renderBackend->CreateRenderDevices(physicalDeviceIDs.data(), numRenderDevices, renderDeviceMasks.data());
	mainRenderDeviceMask = renderDeviceMasks[0];

	return true;
}

void Application::ShutdownRenderBackend()
{
}

/// Temp!!!!
bool LoadShaderBlobFromFile(const String& filename, ShaderBlob* shaderBlob)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		LOG_ERROR("Failed to open shader source file.");
		return false;
	}
	size_t fileSize = (size_t)file.tellg();

	shaderBlob->size = fileSize;
	shaderBlob->data = new unsigned char[fileSize];

	file.seekg(0);
	file.read((char*)shaderBlob->data, fileSize);
	file.close();

	return true;
}

void Application::CreateResources()
{
	ResourceCommandList resourceCommandList;

	RenderBackendShaderDesc pathTracingShaderDesc;
	LoadShaderBlobFromFile("", &pathTracingShaderDesc.stages[(uint32)ShaderStage::Compute]);
	RenderBackendShaderDesc finalShaderDesc;
	LoadShaderBlobFromFile("", &finalShaderDesc.stages[(uint32)ShaderStage::Compute]);

	pathTracingShader = resourceCommandList.CreateShader(mainRenderDeviceMask, pathTracingShaderDesc);
	finalShader = resourceCommandList.CreateShader(mainRenderDeviceMask, finalShaderDesc);

	colorBuffer = resourceCommandList.CreateTexture(mainRenderDeviceMask, RenderBackendTextureDesc::Create2D(
		mainWindow.swapChainWidth,
		mainWindow.swapChainHeight,
		PixelFormat::RGBA16Float,
		TextureCreateFlags::UnorderedAccess | TextureCreateFlags::ShaderResource
	));

	finalBuffer = resourceCommandList.CreateTexture(mainRenderDeviceMask, RenderBackendTextureDesc::Create2D(
		mainWindow.swapChainWidth,
		mainWindow.swapChainHeight,
		PixelFormat::RGBA16Float,
		TextureCreateFlags::UnorderedAccess | TextureCreateFlags::ShaderResource
	));

	renderBackend->SubmitResourceCommandLists(&resourceCommandList, 1);
}

void Application::DestroyResources()
{

}

void Application::UpdateGUI()
{

}

void Application::ResetCurrentSPP()
{
	currentSPP = 0;
}

void Application::Update(float deltaTime)
{
	const float deltaImpulse = 1.0f;
	if (Input::IsKeyPressed(KeyID::W))
	{
		userImpulseData.moveForwardBackwardImpulse += deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyID::S))
	{
		userImpulseData.moveForwardBackwardImpulse -= deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyID::D))
	{
		userImpulseData.moveRightLeftImpulse += deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyID::A))
	{
		userImpulseData.moveRightLeftImpulse -= deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyID::E))
	{
		userImpulseData.moveUpDownImpulse += deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyID::Q))
	{
		userImpulseData.moveUpDownImpulse -= deltaImpulse;
	}

	static Vector2 lastMousePos = { 0.0f, 0.0f };
	const Vector2& mousePos = Input::GetMousePos();
	Vector2 mouseMove = (mousePos - lastMousePos);
	lastMousePos = mousePos;

	if (Input::IsMouseButtonPressed(MouseButtonID::ButtonMiddle))
	{
		userImpulseData.moveRightLeftImpulse += -mouseMove.x * cameraController.config.translationMultiplier;
		userImpulseData.moveUpDownImpulse += mouseMove.y * cameraController.config.translationMultiplier;
	}
	else if (Input::IsMouseButtonPressed(MouseButtonID::ButtonRight))
	{
		userImpulseData.rotatePitchImpulse += -mouseMove.y * cameraController.config.rotationMultiplier;
		userImpulseData.rotateYawImpulse += -mouseMove.x * cameraController.config.rotationMultiplier;
	}

	const float cameraBoost = Input::IsKeyPressed(KeyID::LeftShift) ? 2.0f : 1.0f;
	const float finalCameraSpeed = cameraSpeed * cameraBoost;

	cameraController.Update(userImpulseData, deltaTime, finalCameraSpeed, camera.position, camera.euler);

	userImpulseData.Reset();

	{
		const float spin_speed = 0.1f * 2.f * TM_PI * dt;
		const tm_vec4_t q = tm_quaternion_from_rotation((tm_vec3_t){ 0, 1, 0 }, spin_speed);
		app->object_transform.rot = tm_quaternion_mul(q, app->object_transform.rot);
	}

	camera.aspectRatio = (float)width / (float)height;
	camera.Update();
}

void Application::Render()
{
	RenderCommandList* commandList;
	renderBackendInterface->AllocateRenderCommandList(renderBackendInstance, &commandList, 1);

	RenderBackendTextureHandle backBuffer = renderBackendInterface->GetActiveSwapChainBackBuffer(renderBackendInstance, mainWindow.swapChain);

	uint32 width = mainWindow.swapChainWidth;
	uint32 height = mainWindow.swapChainHeight;

	uint32 dispatchWidth = CEIL_DIV(width, 8);
	uint32 dispatchHeight = CEIL_DIV(height, 8);

	SceneViewData sceneViewData = {
		.cameraPosition = camera.position,
		.viewMatrix = camera.viewMatrix,
		.invViewMatrix = Math::Inverse(camera.viewMatrix),
		.projectionMatrix = camera.projectionMatrix,
		.invProjectionMatrix = Math::Inverse(camera.viewMatrix),
		.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix,
	};

	commandList.UpdateBuffer(sceneViewDataBuffer, (const void*)&sceneViewData, sizeof(SceneViewData), 0);
	{
		if (currentSPP < maxSPP)
		{	
			commandList.Dispatch2D(
				pathTracingShader,
				,
				dispatchWidth,
				dispatchHeight
			);
			currentSPP++;
		}

		commandList.Transitions(
			{ ResourceTransition(colorBuffer, { 0, 1, 0, 1 }, ResourceState::UnorderedAccess, 						ResourceState::ShaderResource),
			  ResourceTransition(finalBuffer, { 0, 1, 0, 1 }, ResourceState::Undefined, ResourceState::UnorderedAccess) }
		);

		renderCommandList.Dispatch2D(
			finalShader,
			dispatchWidth,
			dispatchHeight
		);

		renderCommandList.Transitions(
			{ ResourceTransition(finalBuffer, { 0, 1, 0, 1 }, ResourceState::UnorderedAccess, ResourceState::CopySrc),
				ResourceTransition(colorBuffer, { 0, 1, 0, 1 }, ResourceState::ShaderResource, ResourceState::UnorderedAccess),
				ResourceTransition(backbuffer, { 0, 1, 0, 1 }, ResourceState::Undefined, ResourceState::CopyDst) }
		);

		commandList.CopyTexture(
			finalBuffer,
			extent,
			mainWindow.swapChain,
			extent,
			TextureFilter::Linear
		);

		commandList.Transitions(
			{ ResourceTransition(backBuffer, { 0, 1, 0, 1 }, ResourceState::CopyDst, ResourceState::Present) }
		);
	}
	
	renderBackendInterface->SubmitRenderCommandLists(renderBackendInstance, &commandList, 1);
	renderBackendInterface->FreeRenderCommandLists(renderBackendInstance, &commandList, 1);
}

Application* CreateApplication(int argc, char** argv)
{
	ASSERT(!gApplication);

	char exePath[100];
	GetExePath(exePath, sizeof(exePath));
	
	String exeDirectory = GetDirectory(String(exePath));
	
	// Load plugins
	{
		const String pluginDirectory = exeDirectory + String("plugins");
		PluginSystem::LoadAllPlugins(pluginDirectory.c_str());
	}

	Application* app = new Application();

	// Init application
	{
		app->name = APPLICATION_NAME;
		app->version = APPLICATION_VERSION;

		gApplication = app;
	}

	ASSERT(app->InitRenderBackend());
	
	app->InitMainWindow();

	return app;
}

void DestroyApplication(Application* app)
{
	ASSERT(app && (gApplication == app));
	
	app->ShutdownMainWindow();

	app->ShutdownRenderBackend();
	
	delete app;

	gApplication = nullptr;
}

bool TickApplication(Application* app)
{
	app->renderBackend->BeginFrame(app->renderBackendInstance);

	float now = Time::Now();
	float deltaTime = ;

	bool isAppFocused = false;
	bool isAppMinimized = true;
	{
		if (app->mainWindow.focused)
		{
			isAppFocused = true;
		}
		if (!app->mainWindow.minimized)
		{
			isAppMinimized = false;
		}
	}
	bool isAppThrottled = !isAppFocused || isAppMinimized;

	if (isAppThrottled)
	{
		SuspendCurrentThread(0.1f);
	}

	glfwPollEvents();

	if (glfwWindowShouldClose(app->mainWindow.handle))
	{
		app->exit = true;
		return false;
	}

    app->Update(deltaTime);

	if (!glfwWindowShouldClose(app->mainWindow.handle) && !app->mainWindow.minimized)
	{
		// Resize swap chain if needed
		uint32 width = app->mainWindow.width;
		uint32 height = app->mainWindow.height;
		if (width != app->mainWindow.swapChainWidth || height != app->mainWindow.swapChainHeight)
		{
			app->renderBackendInterface->ResizeSwapChain(app->renderBackend, app->mainWindow.swapChain, &app->mainWindow.swapChainWidth, &app->mainWindow.swapChainHeight);
		}
	}
	
	if (!glfwWindowShouldClose(app->mainWindow.handle) && !app->mainWindow.minimized)
	{
		app->Render();
		app->renderBackendInterface->PresentSwapChain(app->renderBackend, app->mainWindow.swapChain);
	}

	app->renderBackend->EndFrame();
	app->frameCounter++;

	if (glfwWindowShouldClose(app->mainWindow.handle))
	{
		app->exit = true;
	}

	return !app->exit;
}
}