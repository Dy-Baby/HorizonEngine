import Horizon.Core;
import Horizon.Render.Core;

namespace HE
{
	/**
	 * Scene management at runtime.
	 */
	class SceneManager
	{
	public:
		static uint32 kLoadedSceneCount;
		static Scene* kActiveScene;
		static std::map<String, SharedPtr<Scene>> kSceneMapByName;
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