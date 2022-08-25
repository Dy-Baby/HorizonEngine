module HorizonEngine.SceneManagement;

namespace HE
{
	uint32 SceneManager::kLoadedSceneCount = 0;
	Scene* SceneManager::kActiveScene = nullptr;
	std::map<std::string, std::shared_ptr<Scene>> SceneManager::kSceneMapByName;

	Scene* SceneManager::CreateScene(const std::string& name)
	{
		auto scene = new Scene();
		kSceneMapByName[name] = (std::shared_ptr<Scene>)scene;
		return scene;
	}

	Scene* SceneManager::GetActiveScene()
	{
		return kActiveScene;
	}

	Scene* SceneManager::GetSceneByName(const std::string& name)
	{
		// TODO
		return nullptr;
	}

	void SceneManager::SetActiveScene(Scene* scene)
	{
		kActiveScene = scene;
	}

	void SceneManager::LoadScene(const std::string& filename)
	{
		// TODO
	}

	void SceneManager::LoadSceneAsync(const std::string& filename)
	{
		// TODO
	}

	void SceneManager::UnloadSceneAsync(Scene* scene)
	{
		// TODO
	}

	void SceneManager::MergeScenes(Scene* dstScene, Scene* srcScene)
	{
		// TODO
	}
}