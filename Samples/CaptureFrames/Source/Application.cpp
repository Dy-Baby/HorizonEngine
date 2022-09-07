#include "Application.h"
#include "MainWindow.h"
#include "HybridRenderPipeline/HybridRenderPipeline.h"

#include <chrono>

import HorizonEngine.Render.VulkanRenderBackend;
import HorizonEngine.Input;

namespace HE
{
	Application* Application::Instance = nullptr;

	Application::Application()
		: isExitRequested(false)
		, frameCounter(0)
	{
		ASSERT(!Instance);
		Instance = this;
	}

	Application::~Application()
	{
		ASSERT(Instance);
		Instance = nullptr;
	}

	void SetupEarthAtmosphere(SkyAtmosphereComponent* component)
	{
		// Values shown here are the result of integration over wavelength power spectrum integrated with paricular function.
		// Refer to https://github.com/ebruneton/precomputed_atmospheric_scattering for details.

		// All units in kilometers
		const float earthRadius = 6360.0f;
		const float earthAtmosphereHeight = 100.0;   // 100km atmosphere radius, less edge visible and it contain 99.99% of the atmosphere medium https://en.wikipedia.org/wiki/K%C3%A1rm%C3%A1n_line
		const float earthRayleighScaleHeight = 8.0f;
		const float earthMieScaleHeight = 1.2f;

		const double maxSunZenithAngle = M_PI * 120.0 / 180.0;
		// Earth
		component->groundRadius = earthRadius;
		component->groundAlbedo = { 0.401978f, 0.401978f, 0.401978f };
		component->atmosphereHeight = earthAtmosphereHeight;
		component->multipleScatteringFactor = 1.0;
		// Raleigh
		component->rayleighScattering = { 0.005802f, 0.013558f, 0.033100f }; // 1/km
		component->rayleighScaleHeight = earthRayleighScaleHeight;
		// Mie
		component->mieScattering = { 0.003996f, 0.003996f, 0.003996f }; // 1/km
		component->mieExtinction = { 0.004440f, 0.004440f, 0.004440f }; // 1/km
		component->mieAnisotropy = 0.8f;
		component->mieScaleHeight = earthMieScaleHeight;
		// Absorption
		component->absorptionDensity[0] = { 25.0f, 0.0f, 0.0f, 1.0f / 15.0f, -2.0f / 3.0f };
		component->absorptionDensity[1] = { 0.0f, 0.0f, 0.0f, -1.0f / 15.0f, 8.0f / 3.0f };
		component->absorptionExtinction = { 0.000650f, 0.001881f, 0.000085f }; // 1/km
		component->cosMaxSunZenithAngle = (float)Math::Cos(maxSunZenithAngle);
	}

	bool Application::Init()
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

		//int flags = VULKAN_RENDER_BACKEND_CREATE_FLAGS_SURFACE;
		int flags = VULKAN_RENDER_BACKEND_CREATE_FLAGS_VALIDATION_LAYERS | VULKAN_RENDER_BACKEND_CREATE_FLAGS_SURFACE;
		renderBackend = VulkanRenderBackendCreateBackend(flags);

		uint32 deviceMask;
		uint32 physicalDeviceID = 0;
		RenderBackendCreateRenderDevices(renderBackend, &physicalDeviceID, 1, &deviceMask);

		swapChain = RenderBackendCreateSwapChain(renderBackend, deviceMask, (uint64)window->GetNativeHandle());
		swapChainWidth = window->GetWidth();
		swapChainHeight = window->GetHeight();

		uiRenderer = new UIRenderer(window->handle, renderBackend, shaderCompiler);
		uiRenderer->Init();

		//GLTF2ImportSettings settings;
		//ImportGLTF2("../../../Assets/Models/DamagedHelmet/glTF/DamagedHelmet.gltf", settings, scene);
		//ImportGLTF2("../../../Assets/Models/floor/floor.gltf", settings, scene);
		//ImportGLTF2("../../../Assets/Models/Sponza/glTF/Sponza.gltf", settings, scene);
		
		//sky = entityManager->CreateEntity("Sky");
		//entityManager->AddComponent<TransformComponent>(sky);
		//entityManager->AddComponent<HierarchyComponent>(sky);
		//auto& skyAtmosphereComponent = entityManager->AddComponent<SkyAtmosphereComponent>(sky);
		//SetupEarthAtmosphere(&skyAtmosphereComponent);

		activeScene = SceneManager::CreateScene("DefaultScene");
		SceneManager::SetActiveScene(activeScene);
		
		auto entityManager = activeScene->GetEntityManager();

		mainCamera = entityManager->CreateEntity("MainCamera");
		TransformComponent cameraTransform;
		cameraTransform.position = Vector3(0.0, 0.0, 5.0);
		cameraTransform.rotation = Vector3(0.0, 0.0, 0.0);
		cameraTransform.scale = Vector3(1.0);
		entityManager->AddComponent<TransformComponent>(mainCamera, cameraTransform);
		entityManager->AddComponent<SceneHierarchyComponent>(mainCamera);
		CameraComponent cameraComponent;
		cameraComponent.type = CameraType::Perpective;
		cameraComponent.nearPlane = 0.1;
		cameraComponent.farPlane = 3000.0;
		cameraComponent.fieldOfView = 60.0;
		cameraComponent.aspectRatio = 16.0 / 9.0;
		cameraComponent.overrideAspectRatio = false;
		entityManager->AddComponent<CameraComponent>(mainCamera, cameraComponent);

		auto directionalLight = entityManager->CreateEntity("DirectionalLight");

		DirectionalLightComponent directionalLightComponent;
		directionalLightComponent.color = Vector4(1.0f);
		directionalLightComponent.intensity = 1.0f;
		entityManager->AddComponent<DirectionalLightComponent>(directionalLight, directionalLightComponent);
		entityManager->AddComponent<TransformComponent>(directionalLight);
		entityManager->AddComponent<SceneHierarchyComponent>(directionalLight);

		auto skyLight = entityManager->CreateEntity("SkyLight");
		SkyLightComponent skyLightComponent;
		skyLightComponent.cubemapResolution = 128;
		//skyLightComponent.SetCubemap("../../../Assets/HDRIs/PaperMill_E_3k.hdr");
		skyLightComponent.SetCubemap("../../../Assets/HDRIs/HDR_029_Sky_Cloudy_Ref.hdr");
		entityManager->AddComponent<SkyLightComponent>(skyLight, skyLightComponent);
		entityManager->AddComponent<TransformComponent>(skyLight);
		entityManager->AddComponent<SceneHierarchyComponent>(skyLight);
		
		auto mesh = entityManager->CreateEntity("Mesh");
		StaticMeshComponent staticMeshComponent;
		staticMeshComponent.meshSource = "../../../Assets/Models/Sponza/glTF/Sponza.gltf"; 
		//staticMeshComponent.meshSource = "../../../Assets/Models/SciFiHelmet/glTF/SciFiHelmet.gltf";
		//staticMeshComponent.meshSource = "../../../Assets/Models/DamagedHelmet/glTF/DamagedHelmet.gltf";
		//staticMeshComponent.meshSource = "../../../Assets/Models/SunTemple_v4/SunTemple.gltf";
		entityManager->AddComponent<StaticMeshComponent>(mesh, staticMeshComponent);
		entityManager->AddComponent<TransformComponent>(mesh);
		entityManager->AddComponent<SceneHierarchyComponent>(mesh);

		{
			SceneSerializer serializer(activeScene);
			serializer.Serialize("../../../Assets/Scenes/Sponza.horizon");
		}

		Scene* tScene = SceneManager::CreateScene("Sponza");
		{
			SceneSerializer serializer(tScene);
			serializer.Deserialize("../../../Assets/Scenes/Sponza.horizon");
			serializer.Serialize("../../../Assets/Scenes/DefaultScene.horizon");
		}

		AssimpImporter assimpImporter;
		assimpImporter.ImportAsset(staticMeshComponent.meshSource.c_str());

		selectedEntity = mesh;

		GGlobalShaderLibrary = new ShaderLibrary(renderBackend, shaderCompiler);

		renderScene = new RenderScene(arena);
		renderScene->renderBackend = renderBackend;

		SkyLightComponent* sl = entityManager->TryGetComponent<SkyLightComponent>(skyLight);
		sl->proxy = new SkyLightRenderProxy(sl);
		renderScene->SetSkyLight(sl->proxy);
		renderScene->UploadResources(activeScene);
		//system("pause");

		renderContext = new RenderContext();
		renderContext->arena = arena;
		renderContext->renderBackend = renderBackend;
		renderContext->shaderCompiler = shaderCompiler;
		renderContext->uiRenderer = uiRenderer;

		renderPipeline = new HybridRenderPipeline(renderContext);
		renderPipeline->Init();

		sceneView = new SceneView();
		sceneView->scene = renderScene;

		return true;
	}

	void Application::Exit()
	{
		delete renderScene;
		uiRenderer->Shutdown();
		delete uiRenderer;
		VulkanRenderBackendDestroyBackend(renderBackend);
		DestroyDxcShaderCompiler(shaderCompiler);
		delete window;
		GLFWExit();
		delete arena;
	}

	float Application::CalculateDeltaTime()
	{
		static std::chrono::steady_clock::time_point previousTimePoint{ std::chrono::steady_clock::now() };
		std::chrono::steady_clock::time_point timePoint = std::chrono::steady_clock::now();
		std::chrono::duration<float> timeDuration = std::chrono::duration_cast<std::chrono::duration<float>>(timePoint - previousTimePoint);
		float deltaTime = timeDuration.count();
		previousTimePoint = timePoint;
		return deltaTime;
	}

	void Application::Update(float deltaTime)
	{
		auto& cameraTransform = activeScene->GetEntityManager()->GetComponent<TransformComponent>(mainCamera);
		cameraController.Update(deltaTime, cameraTransform.position, cameraTransform.rotation);

		activeScene->Update(deltaTime);

		uiRenderer->BeginFrame();
		OnImGui();
		uiRenderer->EndFrame();
	}

	void Application::Render()
	{
		auto& camera = activeScene->GetEntityManager()->GetComponent<CameraComponent>(mainCamera);
		auto& cameraTransform = activeScene->GetEntityManager()->GetComponent<TransformComponent>(mainCamera);
		sceneView->renderPipeline = renderPipeline;
		sceneView->target = RenderBackendGetActiveSwapChainBuffer(renderBackend, swapChain);
		sceneView->targetDesc = RenderBackendTextureDesc::Create2D(swapChainWidth, swapChainHeight, PixelFormat::BGRA8Unorm, TextureCreateFlags::Present);
		sceneView->targetWidth = swapChainWidth;
		sceneView->targetHeight = swapChainHeight;
		sceneView->camera.fieldOfView = camera.fieldOfView;
		sceneView->camera.zNear = camera.nearPlane;
		sceneView->camera.zFar = camera.farPlane;
		sceneView->camera.position = cameraTransform.position;
		sceneView->camera.euler = cameraTransform.rotation;
		if (camera.overrideAspectRatio)
		{
			sceneView->camera.aspectRatio = camera.aspectRatio;
		}
		else
		{
			sceneView->camera.aspectRatio = (float)swapChainWidth / (float)swapChainHeight;
		}
		static Quaternion zUpQuat = glm::rotate(glm::quat(), Math::DegreesToRadians(90.0), Vector3(0.0, 1.0, 0.0)) * glm::rotate(glm::quat(), Math::DegreesToRadians(90.0), Vector3(0.0, 0.0, 1.0));
		sceneView->camera.invViewMatrix = Math::Compose(sceneView->camera.position, Quaternion(Math::DegreesToRadians(sceneView->camera.euler)) * zUpQuat, Vector3(1.0f, 1.0f, 1.0f));
		sceneView->camera.viewMatrix = Math::Inverse(sceneView->camera.invViewMatrix);
		sceneView->camera.projectionMatrix = glm::perspectiveRH_ZO(Math::DegreesToRadians(sceneView->camera.fieldOfView), sceneView->camera.aspectRatio, sceneView->camera.zNear, sceneView->camera.zFar);
		sceneView->camera.invProjectionMatrix = Math::Inverse(sceneView->camera.projectionMatrix);
		RenderSceneView(renderContext, sceneView);

		gRenderGraphResourcePool->Tick();

	}

	int Application::Run()
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

			float deltaTime = CalculateDeltaTime();
			Update(deltaTime);

			uint32 width = window->GetWidth();
			uint32 height = window->GetHeight();
			if (width != swapChainWidth || height != swapChainHeight)
			{
				RenderBackendResizeSwapChain(renderBackend, swapChain, &width, &height);
				swapChainWidth = width;
				swapChainHeight = height;
			}

			Render();

			RenderBackendPresentSwapChain(renderBackend, swapChain);

			((LinearArena*)arena)->Reset();

			frameCounter++;
		}
		return 0;
	}

	int ApplicationMain()
	{
		int exitCode = EXIT_SUCCESS;
		Application* app = new Application();
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
}