module;

#include <string>
#include <map>
#include <string>
#include <memory>

export module HorizonEngine.SceneManagement;

import HorizonEngine.Core;

export namespace HE
{
	class Scene
	{

	};

	/**
	 * Scene management at runtime.
	 */
	class SceneManager
	{
	public:
		static uint32 kLoadedSceneCount;
		static Scene* kActiveScene;
		static std::map<std::string, std::shared_ptr<Scene>> kSceneMapByName;
		static Scene* CreateScene(const std::string& sceneName);
		static Scene* GetActiveScene();
		static Scene* GetSceneByName(const std::string& sceneName);
		static void SetActiveScene(Scene* scene);
		static void LoadScene(const std::string& filename);
		static void LoadSceneAsync(const std::string& filename);
		static void UnloadSceneAsync(Scene* scene);
		static void MergeScenes(Scene* dstScene, Scene* srcScene);
	};
}