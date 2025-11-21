#include "kgpch.h"

#include "Modules/Scenes/SceneContext.h"
#include "Modules/Events/SceneEvent.h"
#include "Modules/Particles/ParticleContext.h"

namespace Kargono::Scenes
{
	void SceneContext::Init()
	{
		// TODO: Previously had register has component func
	}

	void SceneContext::Terminate()
	{
		// Custom closing of input spec
		m_RenderSceneSpec.m_Shader = nullptr;
		m_RenderSceneSpec.m_Texture = nullptr;
		m_RenderSceneSpec.m_ShapeComponent = nullptr;
		m_RenderSceneSpec.m_CurrentDrawBuffer = nullptr;

		m_ActiveScene.reset();
		m_ActiveSceneHandle = Assets::k_EmptyHandle;
	}

	bool SceneContext::IsSceneActive(UUID sceneID)
	{
		KG_ASSERT(m_ActiveScene);
		KG_ASSERT(m_ActiveSceneHandle != Assets::k_EmptyHandle);
		return sceneID == m_ActiveSceneHandle;
	}
	void SceneContext::TransitionScene(Assets::AssetHandle newSceneHandle)
	{
	    Assets::AssetRef<Scene> newScene = Assets::s_SceneManager.GetAssetByHandle(newSceneHandle);
		if (!newScene)
		{
			KG_WARN("Could not locate scene by scene handle");
			return;
		}
		TransitionScene(newScene);
		m_ActiveSceneHandle = newSceneHandle;
		Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(newSceneHandle, Events::ManageSceneAction::Open);
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);

	}

	void SceneContext::TransitionScene(Ref<Scene> newScene)
	{
		if (!newScene) { return; }

		Physics::Physics2DService().GetActiveContext().Terminate();
		Physics::Physics2DService().RemovePhysics2DWorld();
		m_ActiveScene->OnRuntimeStop();
		m_ActiveScene->m_EntityRegistry.ClearEntities();
		m_ActiveScene.reset();

		m_ActiveScene = newScene;

		m_ActiveScene->ClearHoveredEntity();
		m_ActiveScene->ClearSelectedEntity();

		Physics::Physics2DService::CreatePhysics2DWorld();
		Physics::Physics2DService::GetActiveContext().Init
		(
			m_ActiveScene.get(),
			m_ActiveScene->m_PhysicsSpecification
		);

		m_ActiveScene->OnRuntimeStart();
	}

	void SceneContext::TransitionSceneFromHandle(Assets::AssetHandle sceneID)
	{
	    Assets::AssetRef<Scenes::Scene> sceneReference = Assets::s_SceneManager.GetAssetByHandle(sceneID);
		if (sceneReference)
		{
			Particles::ParticleService::GetActiveContext().ClearEmitters();
			TransitionScene(sceneReference);

			m_ActiveSceneHandle = sceneID;
			Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(sceneID, Events::ManageSceneAction::Open);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);

			Particles::ParticleService::GetActiveContext().LoadSceneEmitters(sceneReference);
		}
		else
		{
			KG_WARN("Attempt to transition scenes, however, new scene reference could not be found!");
		}
	}

	Ref<Scene> SceneContext::GetActiveScene()
	{
		return m_ActiveScene;
	}

	Assets::AssetHandle SceneContext::GetActiveSceneHandle()
	{
		return m_ActiveSceneHandle;
	}

	void SceneContext::SetActiveScene(Ref<Scene> newScene, Assets::AssetHandle newHandle)
	{
		m_ActiveScene = newScene;
		m_ActiveSceneHandle = newHandle;

		Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(newHandle, Events::ManageSceneAction::Open);
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
	}
}