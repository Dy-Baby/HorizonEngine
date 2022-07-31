#include "Application.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "AssimpImporter/AssimpImporter.h"

namespace HE
{

struct GpuMesh
{
	uint32 numVertices;
	uint32 numIndices;
	RenderBackendBufferHandle indexBuffer;
	RenderBackendBufferHandle vertexBuffer;
};

struct Model
{
	Vector<GpuMesh> meshes;
};

struct FrameData
{
	float32 time;
	Vector3 cameraPosition;
	// Vector3 sunDirection;
	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 invViewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 invProjectionMatrix;
	Matrix4x4 viewProjectionMatrix;
};

enum class WindowState
{
	Normal,
	Minimized,
	Maximized,
	Fullscreen,
};

struct WindowTimer
{
	float64 delta;
	float64 totalTime;
	float64 lastTime;
	float64 nextTime;
	float64 startTime;
	bool sleep;
};

struct Window
{
	GLFWwindow* handle;
	uint32 width;
	uint32 height;
	const char* title;

	WindowState finalState;
	WindowTimer timer;
	bool focused;

	RenderBackendSwapChain swapChain;
	uint32 swapChainWidth;
	uint32 swapChainHeight;
	
	bool IsNormal() const
	{
		return finalState == WindowState::Normal;
	}

	bool IsMinimized() const
	{
		return finalState == WindowState::Minimized;
	}

	bool IsMaximized() const
	{
		return finalState == WindowState::Maximized;
	}

	bool IsFullscreen() const
	{
		return finalState == WindowState::Fullscreen;
	}

	bool ShouldClose() const
	{
		return (bool)glfwWindowShouldClose(handle);
	}
};

class Application
{
public:
	void InitMainWindow();
	void ShutdownMainWindow();
	void InitRenderBackend();
	void ShutdownRenderBackend();
	void CreateResources();
	void DestroyResources();
	void UpdateGUI();
	void ResetCurrentSPP();
	void Update(float deltaTime);
	void Render();
	Window* GetMainWindow() 
	{
		return &mainWindow;
	}
private:
    friend Application* CreateApplication(int argc, char** argv);
	friend void DestroyApplication(Application* app);
	friend bool TickApplication(Application* app);

	MemoryArena* allocator;

	const char* name;
	uint32 version;
	bool exit;

	RenderBackendInterface vulkanRBI;
	RenderBackendShaderCompilerInterface* shaderCompilerInterface;

	RenderBackendInstance renderBackend;
	RenderBackendShaderCompiler shaderCompiler;
	RenderCommandListPool* commandListPool;

	uint32 deviceMask;

	Window mainWindow;
	uint32 initialWindowWidth;
	uint32 initialWindowHeight;

	uint64 frameCounter;

	uint32 currentSPP;
	uint32 maxSPP;

	Model model;
	AssimpImportResult aiResult;
	Transform modelTransform;

	//SceneViewManager* sceneViewManager;
	//SceneView* sceneView;

	float cameraSpeed;
	EditorCamera camera;
	EditorCameraController cameraController;
	EditorCameraControllerUserImpulseData userImpulseData;

	RenderBackendShaderHandle gbufferShader;
	RenderBackendShaderHandle tonemappingShader;

	RenderBackendTextureHandle colorBuffer;
	RenderBackendTextureHandle finalBuffer;
	RenderBackendTextureHandle depthBuffer;

	RenderBackendBufferHandle rgenShaderBindingTable;
	RenderBackendBufferHandle rmissShaderBindingTable;
	RenderBackendBufferHandle rchitShaderBindingTable;

	RenderBackendBufferHandle frameDataBuffer;
};
Application* gApplication = nullptr;

namespace Input
{
bool IsKeyPressed(KeyCode key)
{
	Window* window = gApplication->GetMainWindow();
	int state = glfwGetKey(window->handle, static_cast<int32>(key));
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool IsMouseButtonPressed(MouseButtonID button)
{
	Window* window = gApplication->GetMainWindow();
	int state = glfwGetMouseButton(window->handle, (int)button);
	return state == GLFW_PRESS;
}

void GetMousePos(float32* x, float32* y)
{
	Window* window = gApplication->GetMainWindow();
	double xpos, ypos;
	glfwGetCursorPos(window->handle, &xpos, &ypos);
	*x = (float)xpos;
	*y = (float)ypos;
}
}

void Application::InitMainWindow()
{
	if (glfwInit() != GLFW_TRUE)
	{
		HE_LOG_ERROR("Failed to init glfw.");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	mainWindow.width = initialWindowWidth;
	mainWindow.height = initialWindowHeight;
	mainWindow.title = APPLICATION_NAME;

	mainWindow.handle = glfwCreateWindow(mainWindow.width, mainWindow.height, APPLICATION_NAME, nullptr, nullptr);
	if (mainWindow.handle == nullptr)
	{
		HE_LOG_ERROR("Failed to create glfw window.");
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
	glfwSetWindowSizeCallback(mainWindow.handle, [](GLFWwindow* window, int width, int height)
	{
		glfwSetWindowSize(window, width, height);
		int32 w, h;
		glfwGetWindowSize(window, &w, &h);
		gApplication->mainWindow.width = w;
		gApplication->mainWindow.height = h;
		if (gApplication->mainWindow.width == 0 || gApplication->mainWindow.height == 0)
		{
			gApplication->mainWindow.finalState = WindowState::Minimized;
		}
		else if (glfwGetWindowMonitor(window) != nullptr)
		{
			gApplication->mainWindow.finalState = WindowState::Fullscreen;
		}
		else
		{
			gApplication->mainWindow.finalState = WindowState::Normal;
		}
	});
	glfwSetWindowMaximizeCallback(mainWindow.handle, [](GLFWwindow* window, int maximized)
	{
		if (maximized == GLFW_TRUE)
		{
			gApplication->mainWindow.finalState = WindowState::Maximized;
		}
	});

	HWND nativeWindow = glfwGetWin32Window(mainWindow.handle);

	mainWindow.swapChain = vulkanRBI.CreateSwapChain(renderBackend, deviceMask, (uint64)nativeWindow);
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

void Application::InitRenderBackend()
{
	VulkanRenderBackendInitFlags flags = VulkanRenderBackendInitFlags::Surface | VulkanRenderBackendInitFlags::ValidationLayers;
	renderBackend = VulkanRenderBackendCreateBackend(flags);
	ASSERT(renderBackend);
	GetRenderBackendInterface(&vulkanRBI);
	uint32 physicalDeviceID = 0;
	vulkanRBI.CreateRenderDevices(renderBackend, &physicalDeviceID, 1, &deviceMask);
	shaderCompilerInterface = GetRenderBackendShaderCompilerInterface();
	shaderCompiler = shaderCompilerInterface->CreateShaderCompiler(allocator);
}

void Application::ShutdownRenderBackend()
{
	shaderCompilerInterface->DestroyShaderCompiler(shaderCompiler);
	vulkanRBI.DestroyRenderDevices(renderBackend, &deviceMask, 1);
	VulkanRenderBackendDestroyBackend(renderBackend);
}

void Application::CreateResources()
{
	RenderBackendShaderDesc gbufferShaderDesc;
	gbufferShaderDesc.rasterizationState.cullMode = RasterizationCullMode::None;
	gbufferShaderDesc.rasterizationState.frontFaceCounterClockwise = true;
	gbufferShaderDesc.colorBlendState.numColorAttachments = 1;
	gbufferShaderDesc.depthStencilState = {
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.depthCompareOp = CompareOp::LessOrEqual,
		.stencilTestEnable = false,
	};
	shaderCompilerInterface->CompileShader(
		shaderCompiler,
		"D:/Programming/Projects/Horizon/Shaders/GBuffer.hsf",
		"VertexShaderMain",
		RenderBackendShaderStage::Vertex,
		&gbufferShaderDesc.stages[(uint32)RenderBackendShaderStage::Vertex]);
	shaderCompilerInterface->CompileShader(
		shaderCompiler,
		"D:/Programming/Projects/Horizon/Shaders/GBuffer.hsf",
		"PixelShaderMain",
		RenderBackendShaderStage::Pixel,
		&gbufferShaderDesc.stages[(uint32)RenderBackendShaderStage::Pixel]);
	gbufferShader = vulkanRBI.CreateShader(renderBackend, deviceMask, &gbufferShaderDesc, "GbufferShader");

	RenderBackendShaderDesc tonemappingShaderDesc;
	shaderCompilerInterface->CompileShader(
		shaderCompiler,
		"D:/Programming/Projects/Horizon/Shaders/Tonemapping.hsf",
		"ComputeShaderMain",
		RenderBackendShaderStage::Compute,
		&tonemappingShaderDesc.stages[(uint32)RenderBackendShaderStage::Compute]);
	tonemappingShader = vulkanRBI.CreateShader(renderBackend, deviceMask, &tonemappingShaderDesc, "TonemappingShader");

	RenderBackendBufferDesc frameDataBufferDesc = RenderBackendBufferDesc::CreateByteAddress(sizeof(FrameData));
	RenderBackendTextureDesc colorBufferDesc = RenderBackendTextureDesc::Create2D(
		mainWindow.swapChainWidth,
		mainWindow.swapChainHeight,
		PixelFormat::RGBA16Float,
		TextureCreateFlags::RenderTarget | TextureCreateFlags::ShaderResource);
	RenderBackendTextureDesc finalBufferDesc = RenderBackendTextureDesc::Create2D(
		mainWindow.swapChainWidth,
		mainWindow.swapChainHeight,
		PixelFormat::BGRA8Unorm,
		TextureCreateFlags::UnorderedAccess);
	RenderBackendTextureDesc depthBufferDesc = RenderBackendTextureDesc::Create2D(
		mainWindow.swapChainWidth,
		mainWindow.swapChainHeight,
		PixelFormat::D32Float,
		TextureCreateFlags::DepthStencil);
	frameDataBuffer = vulkanRBI.CreateBuffer(renderBackend, deviceMask, &frameDataBufferDesc, "FrameDataBuffer");
	colorBuffer = vulkanRBI.CreateTexture(renderBackend, deviceMask, &colorBufferDesc, "ColorBuffer");
	finalBuffer = vulkanRBI.CreateTexture(renderBackend, deviceMask, &finalBufferDesc, "FinalBuffer");
	depthBuffer = vulkanRBI.CreateTexture(renderBackend, deviceMask, &depthBufferDesc, "DepthBuffer");

	AssimpImporter::ImportFromFile("D:/Programming/Projects/Horizon/Assets/SciFiHelmet/glTF/SciFiHelmet.gltf", allocator, &aiResult);

	bool first = true;
	for (const auto& mesh : aiResult.meshes)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			break;
		}
		GpuMesh gpuMesh;
		gpuMesh.numVertices = mesh.numVertices;
		gpuMesh.numIndices = mesh.numIndices;
		RenderBackendBufferDesc vertexBufferDesc = RenderBackendBufferDesc::CreateByteAddress(mesh.numVertices * sizeof(Vector3));
		gpuMesh.vertexBuffer = vulkanRBI.CreateBuffer(renderBackend, deviceMask, &vertexBufferDesc, "VertexBuffer");
		RenderBackendBufferDesc indexBufferDesc = RenderBackendBufferDesc::CreateByteAddress(mesh.numIndices * sizeof(uint32));
		gpuMesh.indexBuffer = vulkanRBI.CreateBuffer(renderBackend, deviceMask, &indexBufferDesc, "IndexBuffer");
		vulkanRBI.WriteBuffer(renderBackend, gpuMesh.vertexBuffer, 0, mesh.vertices, mesh.numVertices * sizeof(Vector3));
		vulkanRBI.WriteBuffer(renderBackend, gpuMesh.indexBuffer, 0, mesh.indices, mesh.numIndices * sizeof(uint32));
		model.meshes.push_back(gpuMesh);
	}
	const uint32 handleSize = 32;
	RenderBackendBufferDesc rgenSbtDesc = RenderBackendBufferDesc::CreateShaderBindingTable(handleSize, 1);
	RenderBackendBufferDesc rmissSbtDesc = RenderBackendBufferDesc::CreateShaderBindingTable(handleSize, 2);
	RenderBackendBufferDesc rchitSbtDesc = RenderBackendBufferDesc::CreateShaderBindingTable(handleSize, 1);
	rgenShaderBindingTable = vulkanRBI.CreateBuffer(renderBackend, deviceMask, &rgenSbtDesc, "RayGenShaderBindingTable");
	rmissShaderBindingTable = vulkanRBI.CreateBuffer(renderBackend, deviceMask, &rmissSbtDesc, "MissShaderBindingTable");
	hitShaderBindingTable = vulkanRBI.CreateBuffer(renderBackend, deviceMask, &rchitSbtDesc, "ClosetHitShaderBindingTable");

	// Copy handles
	memcpy(shaderBindingTables.raygen.mapped, shaderHandleStorage.data(), handleSize);
	// We are using two miss shaders, so we need to get two handles for the miss shader binding table
	memcpy(shaderBindingTables.miss.mapped, shaderHandleStorage.data() + handleSizeAligned, handleSize * 2);
	memcpy(shaderBindingTables.hit.mapped, shaderHandleStorage.data() + handleSizeAligned * 3, handleSize);

	vulkanRBI.CreateBottomLevelAS();
	vulkanRBI.CreateTopLevelAS();
}

void Application::Render()
{
	RenderCommandList* commandList = nullptr;
	commandListPool->Allocate(&commandList, 1);

	RenderBackendTextureHandle backBuffer = vulkanRBI.GetActiveSwapChainBackBuffer(renderBackend, mainWindow.swapChain);

	uint32 width = mainWindow.swapChainWidth;
	uint32 height = mainWindow.swapChainHeight;

	Matrix4x4 world = Matrix4x4(1);
	FrameData frameData = {
		.time = Time::Now(),
		.cameraPosition = camera.position,
		.worldMatrix = world,
		.viewMatrix = camera.viewMatrix,
		.invViewMatrix = Math::Inverse(camera.viewMatrix),
		.projectionMatrix = camera.projectionMatrix,
		.invProjectionMatrix = Math::Inverse(camera.projectionMatrix),
		.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix,
	};
	commandList->UpdateBuffer(frameDataBuffer, 0, sizeof(frameData), (void*)&frameData);

	static ResourceBarrier transitions[2] = {};

	ShaderArgument shaderArgument = {
		.buffer = frameDataBuffer,
		.offset = 0,
	};

	commandList->TraceRay(
		dsadas,
		shaderArgument,
		rgenShaderBindingTable, 
		rmissShaderBindingTable, 
		rchitShaderBindingTable,
		width, height, 1);

	transitions[0] = ResourceBarrier(backBuffer, { 0, 1, 0, 1 }, RenderBackendResourceState::Undefined, RenderBackendResourceState::CopyDst);
	transitions[1] = ResourceBarrier(finalBuffer, { 0, 1, 0, 1 }, RenderBackendResourceState::UnorderedAccess, RenderBackendResourceState::CopySrc);
	commandList->Transitions(transitions, ARRAY_SIZE(transitions));
	commandList->CopyTexture2D(
		finalBuffer,
		{ 0, 0 }, 0,
		backBuffer,
		{ 0, 0 }, 0,
		{ width, height });
	transitions[0] = ResourceBarrier(backBuffer, { 0, 1, 0, 1 }, RenderBackendResourceState::CopyDst, RenderBackendResourceState::Present);
	commandList->Transitions(transitions, 1);

	vulkanRBI.SubmitRenderCommandLists(renderBackend, &commandList, 1);

	commandListPool->Free(&commandList, 1);
}

void Application::DestroyResources()
{

}

void Application::UpdateGUI()
{

}

void Application::Update(float deltaTime)
{
	const float deltaImpulse = 0.1f;
	if (Input::IsKeyPressed(KeyCode::W))
	{
		userImpulseData.moveForwardBackwardImpulse += deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyCode::S))
	{
		userImpulseData.moveForwardBackwardImpulse -= deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyCode::D))
	{
		userImpulseData.moveRightLeftImpulse += deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyCode::A))
	{
		userImpulseData.moveRightLeftImpulse -= deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyCode::E))
	{
		userImpulseData.moveUpDownImpulse += deltaImpulse;
	}
	if (Input::IsKeyPressed(KeyCode::Q))
	{
		userImpulseData.moveUpDownImpulse -= deltaImpulse;
	}

	static Vector2 lastMousePos = { 0.0f, 0.0f };
	Vector2 mousePos;
	Input::GetMousePos(&mousePos.x, &mousePos.y);
	Vector2 mouseMove = (mousePos - lastMousePos);
	lastMousePos = mousePos;

	if (Input::IsMouseButtonPressed(MouseButtonID::ButtonMiddle))
	{
		userImpulseData.moveRightLeftImpulse += mouseMove.x * cameraController.config.translationMultiplier;
		userImpulseData.moveUpDownImpulse += -mouseMove.y * cameraController.config.translationMultiplier;
	}
	else if (Input::IsMouseButtonPressed(MouseButtonID::ButtonRight))
	{
		userImpulseData.rotatePitchImpulse += mouseMove.y * cameraController.config.rotationMultiplier;
		userImpulseData.rotateYawImpulse += mouseMove.x * cameraController.config.rotationMultiplier;
	}

	const float cameraBoost = Input::IsKeyPressed(KeyCode::LeftShift) ? 2.0f : 1.0f;
	const float finalCameraSpeed = cameraSpeed * cameraBoost;

	cameraController.Update(userImpulseData, deltaTime, finalCameraSpeed, camera.position, camera.euler);

	userImpulseData.Reset();

	camera.aspectRatio = (float)mainWindow.swapChainWidth / (float)mainWindow.swapChainHeight;

	camera.Update();
}

Application* CreateApplication(int argc, char** argv)
{
	ASSERT(!gApplication);

	//char exePath[100];
	//GetExePath(exePath, sizeof(exePath));
	//String exeDirectory = GetDirectory(String(exePath));

	// Load plugins
	{
		//const String pluginDirectory = exeDirectory + String("plugins");
		//PluginSystem::LoadAllPlugins(pluginDirectory.c_str());
	}

	Application* app = new Application();

	// Init application
	{
		app->name = APPLICATION_NAME;
		app->version = APPLICATION_VERSION;
		app->initialWindowWidth = 1280;
		app->initialWindowHeight = 720;
		app->cameraSpeed = 1.0f;
		app->camera = {
			.fieldOfView = 60.0f,
			.aspectRatio = (float)app->initialWindowWidth / (float)app->initialWindowHeight,
			.zNear = 0.01f,
			.zFar = 1000.0f,
			.position = { 0.0f, 0.0f, -10.0f },
			.euler = { 0.0f, 0.0f, 0.0f },
		};
		app->modelTransform = {
			.translation = { 0, 0, 0, 0 },
			.rotation = { 0, 0, 0, 1 },
			.scale = { 1, 1, 1, 1 },
		};
		app->cameraController.config.translationMultiplier = 0.1f;
		app->userImpulseData.Reset();
		app->allocator = new MallocArean();
		app->commandListPool = new RenderCommandListPool(app->allocator);
		//app->sceneViewManager = new SceneViewManager();
		//app->sceneView = app->sceneViewManager->CreateSceneView();

		gApplication = app;
	}

	app->InitRenderBackend();
	app->InitMainWindow();
	app->CreateResources();

	return app;
}

void DestroyApplication(Application* app)
{
	ASSERT(app && (gApplication == app));
	app->ShutdownRenderBackend();
	app->ShutdownMainWindow();
	delete app;
	gApplication = nullptr;
}

void ProcessEvents(Application* app)
{
	// ASSERT(IsInMainThread());
	glfwPollEvents();
}

bool TickApplication(Application* app)
{
	ProcessEvents(app);

	if (app->mainWindow.ShouldClose())
	{
		app->exit = true;
		return !app->exit;
	}

	static float prev = 0.0f;
	float curr = Time::Now();
	float deltaTime = curr - prev;
	prev = curr;
    app->Update(deltaTime);

	app->vulkanRBI.BeginFrame(app->renderBackend);

	if (!app->mainWindow.IsMinimized())
	{
		// Resize swap chain if needed
		uint32 width = app->mainWindow.width;
		uint32 height = app->mainWindow.height;
		if (width != app->mainWindow.swapChainWidth || height != app->mainWindow.swapChainHeight)
		{
			app->vulkanRBI.ResizeSwapChain(app->renderBackend, app->mainWindow.swapChain, &app->mainWindow.swapChainWidth, &app->mainWindow.swapChainHeight);
		}
	}
	
	if (!app->mainWindow.IsMinimized())
	{
		app->Render();
		app->vulkanRBI.PresentSwapChain(app->renderBackend, app->mainWindow.swapChain);
	}

	app->vulkanRBI.EndFrame(app->renderBackend);
	app->frameCounter++;

	return !app->exit;
}

}