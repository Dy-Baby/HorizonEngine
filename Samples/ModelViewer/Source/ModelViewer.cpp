#include "ModelViewer.h"
#include "MainWindow.h"

ModelViewerApp* ModelViewerApp::Instance = nullptr;

ModelViewerApp::ModelViewerApp()
	: isExitRequested(false)
	, frameCounter(0)
{
	ASSERT(!Instance);
	Instance = this;
}

ModelViewerApp::~ModelViewerApp()
{
	ASSERT(Instance);
	Instance = nullptr;
}

bool ModelViewerApp::Init()
{
	using namespace HE;

	arena = new LinearArena(nullptr, 1048576);

	uint32 initialWindowWidth = 1920;
	uint32 initialWindowHeight = 1080;
	
	GLFWInit();
	WindowCreateInfo windowInfo = {
		.width = initialWindowWidth,
		.height = initialWindowHeight,
		.title = HE_APPLICATION_NAME,
		.flags = WindowFlags::Resizable
	};
	window = new MainWindow(&windowInfo);
	Input::SetCurrentContext(window->handle);

	shaderCompiler = CreateDxcShaderCompiler();

	int flags = VULKAN_RENDER_BACKEND_CREATE_VALIDATION_LAYERS_BIT | VULKAN_RENDER_BACKEND_CREATE_SURFACE_BIT;
	renderBackend = VulkanRenderBackendCreateBackend(flags);
	
	uint32 deviceMask;
	uint32 physicalDeviceID = 0;
	CreateRenderDevices(renderBackend, &physicalDeviceID, 1, &deviceMask);
	
	swapChain = CreateSwapChain(renderBackend, deviceMask, (uint64)window->GetNativeHandle());
	swapChainWidth = window->GetWidth();
	swapChainHeight = window->GetHeight();

	uiRenderer = new UIRenderer(window->handle, renderBackend, shaderCompiler);
	uiRenderer->Init();

	scene = new Scene();

	GLTF2ImportSettings settings;
	ImportGLTF2("D:/Programming/HorizonTest/Assets/DamagedHelmet/glTF/DamagedHelmet.gltf", settings, scene);
	//ImportGLTF2("D:/Programming/HorizonTest/Assets/Sponza/glTF/Sponza.gltf", settings, scene);
	scene->renderBackend = renderBackend;
	scene->UploadResources();

	auto entityManager = scene->GetEntityManager();

	auto directionalLight = entityManager->CreateEntity("Directional Light");
	LightComponent lightComponent = {
		.type = LightType::Directional,
		.color = Vector4(1.0f),
		.intensity = 1.0f,
		.direction = Vector4(),
	};
	entityManager->AddComponent<LightComponent>(directionalLight, lightComponent);
	entityManager->AddComponent<TransformComponent>(directionalLight);
	entityManager->AddComponent<HierarchyComponent>(directionalLight);

	CameraComponent cameraComponent = {
		.type = CameraType::Perpective,
		.nearPlane = 0.1,
		.farPlane = 3000.0,
		.fieldOfView = 60.0,
	};
	
	TransformComponent cameraTransform = {
		.position = Vector3(5.0, 0.0, 0.0),
		.rotation = Vector3(0.0, 0.0, 0.0),
		.scale = Vector3(1.0),
	};
	mainCamera = entityManager->CreateEntity("Main Camera");
	entityManager->AddComponent<CameraComponent>(mainCamera, cameraComponent);
	entityManager->AddComponent<TransformComponent>(mainCamera, cameraTransform);
	entityManager->AddComponent<HierarchyComponent>(mainCamera);

	renderContext = new RenderContext();
	renderContext->arena = arena;
	renderContext->renderBackend = renderBackend;
	renderContext->shaderCompiler = shaderCompiler;
	renderContext->uiRenderer = uiRenderer;

	renderPipeline = new DefaultRenderPipeline(renderContext);
	renderPipeline->Init();

	sceneView = new SceneView();
	sceneView->scene = scene;

	return true;
}

void ModelViewerApp::Exit()
{
	delete scene;
	uiRenderer->Shutdown();
	delete uiRenderer;
	VulkanRenderBackendDestroyBackend(renderBackend);
	DestroyDxcShaderCompiler(shaderCompiler);
	delete window;
	GLFWExit();
	delete arena;
}

void ModelViewerApp::Update(float deltaTime)
{
	auto& cameraTransform = scene->GetEntityManager()->GetComponent<HE::TransformComponent>(mainCamera);
	cameraController.Update(deltaTime, cameraTransform.position, cameraTransform.rotation);
	
	scene->Update();
}

void ModelViewerApp::Render()
{
	uiRenderer->BeginFrame();
	OnImGui();
	uiRenderer->EndFrame();

	auto& camera = scene->GetEntityManager()->GetComponent<HE::CameraComponent>(mainCamera);
	camera.aspectRatio = (float)swapChainWidth / (float)swapChainHeight;
	auto& cameraTransform = scene->GetEntityManager()->GetComponent<HE::TransformComponent>(mainCamera);
	sceneView->renderPipeline = renderPipeline;
	sceneView->target = HE::GetActiveSwapChainBuffer(renderBackend, swapChain);
	sceneView->targetDesc = HE::RenderBackendTextureDesc::Create2D(swapChainWidth, swapChainHeight, HE::PixelFormat::BGRA8Unorm, HE::TextureCreateFlags::Present);
	sceneView->targetWidth = swapChainWidth;
	sceneView->targetHeight = swapChainHeight;
	sceneView->camera.aspectRatio = camera.aspectRatio;
	sceneView->camera.fieldOfView = camera.fieldOfView;
	sceneView->camera.zNear = camera.nearPlane;
	sceneView->camera.zFar = camera.farPlane;
	sceneView->camera.position = cameraTransform.position;
	sceneView->camera.euler = cameraTransform.rotation;
	static HE::Quaternion zUpQuat= glm::rotate(glm::quat(), HE::Math::DegreesToRadians(90.0), HE::Vector3(0.0, 1.0, 0.0)) * glm::rotate(glm::quat(), HE::Math::DegreesToRadians(90.0), HE::Vector3(0.0, 0.0, 1.0));
	sceneView->camera.invViewMatrix = HE::Math::Compose(sceneView->camera.position, HE::Quaternion(HE::Math::DegreesToRadians(sceneView->camera.euler)) * zUpQuat, HE::Vector3(1.0f, 1.0f, 1.0f));
	sceneView->camera.viewMatrix = HE::Math::Inverse(sceneView->camera.invViewMatrix);
	sceneView->camera.projectionMatrix = glm::perspectiveRH_ZO(HE::Math::DegreesToRadians(camera.fieldOfView), camera.aspectRatio, camera.nearPlane, camera.farPlane);
	sceneView->camera.invProjectionMatrix = HE::Math::Inverse(sceneView->camera.projectionMatrix);
	RenderSceneView(renderContext, sceneView);

	HE::gRenderGraphResourcePool->Tick();

}

int ModelViewerApp::Run()
{
	while (!IsExitRequest())
	{
		window->ProcessEvents();

		if (window->ShouldClose())
		{
			isExitRequested = true;
		}

		WindowState state = window->GetState();
	
		if (state == WindowState::Minimized)
		{
			continue;
		}

		static std::chrono::steady_clock::time_point previousTimePoint{ std::chrono::steady_clock::now() };
		std::chrono::steady_clock::time_point timePoint = std::chrono::steady_clock::now();
		std::chrono::duration<float> timeDuration = std::chrono::duration_cast<std::chrono::duration<float>>(timePoint - previousTimePoint);
		float deltaTime = timeDuration.count();
		previousTimePoint = timePoint;

		Update(deltaTime);

		uint32 width = window->GetWidth();
		uint32 height = window->GetHeight();
		if (width != swapChainWidth || height != swapChainHeight)
		{
			HE::ResizeSwapChain(renderBackend, swapChain, &width, &height);
			swapChainWidth = width;
			swapChainHeight = height;
		}

		Render();

		HE::PresentSwapChain(renderBackend, swapChain);

		((HE::LinearArena*)arena)->Reset();

		frameCounter++;
	}
	return 0;
}

int ModelViewerMain()
{
	int exitCode = EXIT_SUCCESS;
	ModelViewerApp* app = new ModelViewerApp();
	bool result = app->Init();
	if (result)
	{
		exitCode = app->Run();
	}
	else
	{
		exitCode = EXIT_FAILURE;
	}
	app->Exit();
	delete app;
	return exitCode;
}
