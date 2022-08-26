#include "Application.h"
#include "HybridRenderPipeline/HybridRenderPipeline.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace HE
{
	std::unordered_map<std::string, std::function<void(std::string, void*)>> uiCreator;
	std::vector<std::pair<std::string, bool>> g_editor_node_state_array;
	int                                       g_node_depth = -1;
	bool inited = false;

	void UIInit()
	{
		using namespace HE;
		uiCreator["float"] = [](const std::string& name, void* value)
		{
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::DragFloat(("##" + name).c_str(), static_cast<float*>(value));
			ImGui::PopItemWidth();
			ImGui::NextColumn();
		};

		uiCreator["struct glm::vec<3,float,0>"] = [](const std::string& name, void* value)
		{
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(name.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			ImGui::DragFloat3(("##" + name).c_str(), static_cast<float*>(value));
			ImGui::PopItemWidth();
			ImGui::NextColumn();
		};
	}

	void BeginDockSpace()
	{
		static bool dockSpaceOpen = true;

		// Imgui dock node flags.
		static ImGuiDockNodeFlags dockNodeflags = ImGuiDockNodeFlags_PassthruCentralNode;

		// Imgui window flags.
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking;

		static bool isFullscreenPersistant = true;
		bool isFullscreen = isFullscreenPersistant;
		if (isFullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		windowFlags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		ImGui::Begin("Dockspace Demo", &dockSpaceOpen, windowFlags);
		ImGui::PopStyleVar();

		if (isFullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		// Set min width
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 300.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockSpaceID = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockSpaceID, ImVec2(0.0f, 0.0f), dockNodeflags);
		}
		style.WindowMinSize.x = minWinSizeX;
	}

	void EndDockSpace()
	{
		ImGui::End();
	}

	void DrawOverlay()
	{
		static int corner = 0;
		ImGuiIO& io = ImGui::GetIO();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
		if (corner != -1)
		{
			const float PAD = 10.0f;
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
			ImVec2 work_size = viewport->WorkSize;
			ImVec2 window_pos, window_pos_pivot;
			window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
			window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
			window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
			window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
			ImGui::SetNextWindowViewport(viewport->ID);
			window_flags |= ImGuiWindowFlags_NoMove;
		}
		ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
		static bool p_open = true;
		if (ImGui::Begin("Overlay", &p_open, window_flags))
		{
			ImGui::Text("Horizon Engine");
			ImGui::Separator();
			ImGui::Text("FPS: %.1f (%.2f ms/frame)", ImGui::GetIO().Framerate, (1000.0f / ImGui::GetIO().Framerate));
		}
		ImGui::End();
	}

	template<typename ComponentType>
	void DrawComponent(const ComponentType& component)
	{
		using namespace entt;
		if (ImGui::CollapsingHeader("Sky Atmosphere"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::Columns(2);
			ImGui::Separator();

			for (auto data : entt::resolve<ComponentType>().data())
			{
				std::string type = std::string(data.type().info().name());
				std::string name = data.prop("Name"_hs).value().cast<std::string>();
				auto g = data.get(component);
				void* value = data.get(component).data();
				uiCreator[type](name, value);
			}

			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::PopStyleVar();
		}
	}

	void Application::OnImGui()
	{
		using namespace HE;
		using namespace entt;

		BeginDockSpace();

		if (!inited)
		{
			UIInit();
			inited = true;
		}

		ImGui::Begin("Settings");
		{
			auto& skyAtmosphereComponent = scene->GetEntityManager()->GetComponent<SkyAtmosphereComponent>(sky);
			// DrawComponent<SkyAtmosphereComponent>(skyAtmosphereComponent);

			if (ImGui::CollapsingHeader("Sky Atmosphere"))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
				ImGui::Columns(2);
				ImGui::Separator();

				for (auto data : entt::resolve<SkyAtmosphereComponent>().data())
				{
					std::string type = std::string(data.type().info().name());
					std::string name = data.prop("Name"_hs).value().cast<std::string>();
					auto g = data.get(skyAtmosphereComponent);
					void* value = data.get(skyAtmosphereComponent).data();
					uiCreator[type](name, value);
				}

				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::PopStyleVar();
			}

#if 0
			if (ImGui::CollapsingHeader("Sky Atmosphere"))
			{
				ImGui::Separator();
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
				ImGui::Columns(2);
				ImGui::Separator();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Ground Radius");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat("##Ground Radius", &skyAtmosphereComponent.groundRadius, 0.1f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Ground Albedo");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat3("##Ground Albedo", &skyAtmosphereComponent.groundAlbedo.x, 0.1f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Atmosphere Height");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat("##Atmosphere Height", &skyAtmosphereComponent.atmosphereHeight, 0.1f, 0.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Multiple Scattering Factor");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat("##Multiple Scattering Factor", &skyAtmosphereComponent.multipleScatteringFactor, 0.01f, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Rayleigh Scattering");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat3("##Rayleigh Scattering", &skyAtmosphereComponent.rayleighScattering.x, 0.01f, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Rayleigh Scale Height");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat("##Rayleigh Scale Height", &skyAtmosphereComponent.rayleighScaleHeight, 0.01f, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Mie Scattering");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat3("##Mie Scattering", &skyAtmosphereComponent.mieScattering.x, 0.01f, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Mie Extinction");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat3("##Mie Extinction", &skyAtmosphereComponent.mieExtinction.x, 0.01f, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Mie Anisotropy");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat("##Mie Anisotropy", &skyAtmosphereComponent.mieAnisotropy, 0.01f, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Mie Scale Height");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat("##Mie Scale Height", &skyAtmosphereComponent.mieScaleHeight, 0.01f, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::AlignTextToFramePadding();
				ImGui::TextUnformatted("Absorption Extinction");
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);
				ImGui::DragFloat3("##Absorption Extinction", &skyAtmosphereComponent.absorptionExtinction.x, 0.01f, 0.0f, 1.0f);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::Columns(1);
				ImGui::Separator();
				ImGui::PopStyleVar();
			}
#endif
		}
		ImGui::End();

		DrawOverlay();

		EndDockSpace();
	}
}