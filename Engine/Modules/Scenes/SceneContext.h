#pragma once

#include "Modules/Scenes/Assets/Scene.h"

namespace Kargono::Scenes
{
	class SceneContext
	{
	public:
		//====================
		// Constructors/Destructors
		//====================
		SceneContext() = default;
		~SceneContext() = default;
	public:
		//====================
		// LifeCycle Functions
		//====================
		void Init();
		void Terminate();
	public:
		//====================
		// Query Active Scene
		//====================
		bool IsSceneActive(UUID sceneID);
	public:
		//====================
		// Set New Scene
		//====================
		void TransitionScene(Assets::AssetHandle newSceneHandle);
		void TransitionScene(Ref<Scene> newScene);
		void TransitionSceneFromHandle(Assets::AssetHandle sceneID);
	public:
		//====================
		// Getters/Setters
		//====================
		Ref<Scene> GetActiveScene();
		Assets::AssetHandle GetActiveSceneHandle();
		void SetActiveScene(Ref<Scene> newScene, Assets::AssetHandle newHandle);
	public:
		//====================
		// Public Fields
		//====================
		// Rendering
		Rendering::RendererInputSpec m_RenderSceneSpec{};
	private:
		//====================
		// Internal Fields
		//====================
		Ref<Scene> m_ActiveScene{ nullptr };
		Assets::AssetHandle m_ActiveSceneHandle{ Assets::k_EmptyHandle };
	};

	class SceneService // TODO: EWWWWW UGHHHHHHH
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static SceneContext& GetActiveContext() { return s_SceneContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline SceneContext s_SceneContext{};
	};
}