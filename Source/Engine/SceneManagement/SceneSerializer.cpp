module;

#include <fstream>
#include <functional>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

#include "ECS/ECS.h"

module HorizonEngine.SceneManagement;

namespace YAML
{
	template<>
	struct YAML::convert<HE::Vector2>
	{
		static YAML::Node encode(const HE::Vector2& rhs)
		{
			YAML::Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const YAML::Node& node, HE::Vector2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct YAML::convert<HE::Vector3>
	{
		static YAML::Node encode(const HE::Vector3& rhs)
		{
			YAML::Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, HE::Vector3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
			{
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct YAML::convert<HE::Vector4>
	{
		static YAML::Node encode(const HE::Vector4& rhs)
		{
			YAML::Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const YAML::Node& node, HE::Vector4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct YAML::convert<glm::quat>
	{
		static YAML::Node encode(const HE::Quaternion& rhs)
		{
			YAML::Node node;
			node.push_back(rhs.w);
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const YAML::Node& node, HE::Quaternion& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
			{
				return false;
			}
			rhs.w = node[0].as<float>();
			rhs.x = node[1].as<float>();
			rhs.y = node[2].as<float>();
			rhs.z = node[3].as<float>();
			return true;
		}
	};

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const HE::Vector2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const HE::Vector3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}


	inline YAML::Emitter& operator<<(YAML::Emitter& out, const HE::Vector4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const HE::Quaternion& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.w << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
}

namespace HE
{
	bool IsSerializeAnyInitialized = false;
	std::unordered_map<std::string, std::function<void(YAML::Emitter&, const std::string&, const entt::meta_any&)>> SerializeAny;
	
	void InitializeSerializeAny()
	{
		SerializeAny["int"] = [](YAML::Emitter& out, const std::string& name, const entt::meta_any& value)
		{
			out << YAML::Key << name << YAML::Value << value.cast<int>();
		};

		SerializeAny["float"] = [](YAML::Emitter& out, const std::string& name, const entt::meta_any& value)
		{
			out << YAML::Key << name << YAML::Value << value.cast<float>();
		};

		SerializeAny["struct glm::vec<2,float,0>"] = [](YAML::Emitter& out, const std::string& name, const entt::meta_any& value)
		{
			out << YAML::Key << name << YAML::Value << value.cast<Vector2>();
		};

		SerializeAny["struct glm::vec<3,float,0>"] = [](YAML::Emitter& out, const std::string& name, const entt::meta_any& value)
		{
			out << YAML::Key << name << YAML::Value << value.cast<Vector3>();
		};

		SerializeAny["struct glm::vec<4,float,0>"] = [](YAML::Emitter& out, const std::string& name, const entt::meta_any& value)
		{
			out << YAML::Key << name << YAML::Value << value.cast<Vector4>();
		};

	}

	void SceneSerializer::Serialize(const std::filesystem::path& filename)
	{
		if (!IsSerializeAnyInitialized)
		{
			InitializeSerializeAny();
			IsSerializeAnyInitialized = true;
		}

		YAML::Emitter out;
		out << YAML::BeginMap; // Scene
		{
			out << YAML::Key << "Scene";
			out << YAML::Value << scene->name;

			out << YAML::Key << "Entities";
			out << YAML::Value << YAML::BeginSeq; // Entities

			auto view = scene->entityManager->GetView<NameComponent>();

			for (auto entity : view)
			{
				using namespace entt;
				auto& name = scene->entityManager->GetComponent<NameComponent>(entity);
				out << YAML::BeginMap; // Entity
				out << YAML::Key << "Entity";
				out << YAML::Value << name.name;

				if (scene->entityManager->HasComponent<TransformComponent>(entity))
				{
					const auto& transform = scene->entityManager->GetComponent<TransformComponent>(entity);

					out << YAML::Key << "TransformComponent";
					out << YAML::BeginMap;

					for (auto data : entt::resolve<TransformComponent>().data())
					{
						std::string type = std::string(data.type().info().name());
						std::string name = data.prop("Name"_hs).value().cast<std::string>();
						auto value = data.get(transform);
						SerializeAny[type](out, name, value);
					}

					out << YAML::EndMap;
				}
				if (scene->entityManager->HasComponent<CameraComponent>(entity))
				{
					const auto& camera = scene->entityManager->GetComponent<CameraComponent>(entity);

					out << YAML::Key << "CameraComponent";
					out << YAML::BeginMap;

					for (auto data : entt::resolve<CameraComponent>().data())
					{
						std::string type = std::string(data.type().info().name());
						std::string name = data.prop("Name"_hs).value().cast<std::string>();
						auto value = data.get(camera);
						SerializeAny[type](out, name, value);
					}

					out << YAML::EndMap;
				}

				out << YAML::EndMap; // Entity
			}

			out << YAML::EndSeq; // Entities
		}
		out << YAML::EndMap; // Scene

		std::ofstream fout(filename);
		fout << out.c_str();
	}

	bool SceneSerializer::Deserialize(const std::filesystem::path& filename)
	{
		std::ifstream stream(filename);
		ASSERT(stream);
		std::stringstream sstream;
		sstream << stream.rdbuf();

		YAML::Node data = YAML::Load(sstream.str());
		if (!data["Scene"])
		{
			return false;
		}

		std::string sceneName = data["Scene"].as<std::string>();
		HE_LOG_INFO("Deserializing scene '{0}'", sceneName);

		if (sceneName == "UntitledScene")
		{
			std::filesystem::path path = filename;
			sceneName = path.stem().string();
		}

		scene->name = sceneName;

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entityValue : entities)
			{
				std::string name = entityValue["Entity"].as<std::string>();

				EntityHandle entity = scene->entityManager->CreateEntity(name.c_str());

				auto transformComponent = entityValue["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transforms
					auto& transform = scene->entityManager->AddComponent<TransformComponent>(entity);
					transform.position = transformComponent["Position"].as<Vector3>();
					transform.rotation = transformComponent["Rotation"].as<Vector3>();
					transform.scale = transformComponent["Scale"].as<Vector3>();
				}

				auto cameraComponent = entityValue["CameraComponent"];
				if (cameraComponent)
				{
					auto& camera = scene->entityManager->AddComponent<CameraComponent>(entity);
					camera.nearPlane = cameraComponent["Near Plane"].as<float>();
					camera.farPlane = cameraComponent["Far Plane"].as<float>();
					camera.fieldOfView = cameraComponent["Field of View"].as<float>();
					camera.aspectRatio = cameraComponent["Aspect Ratio"].as<float>();
				}

				auto directionalLightComponent = entityValue["DirectionalLightComponent"];
				if (directionalLightComponent)
				{
					auto& directionalLight = scene->entityManager->AddComponent<DirectionalLightComponent>(entity);
					directionalLight.intensity = directionalLightComponent["Intensity"].as<float>(1.0f);
					directionalLight.color = directionalLightComponent["Color"].as<Vector3>(Vector3(1.0f));
				}
			}
		}
		return true;
	}
}