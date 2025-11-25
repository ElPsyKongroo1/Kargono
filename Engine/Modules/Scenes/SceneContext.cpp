#include "kgpch.h"

#include "Modules/Scenes/SceneContext.h"
#include "Modules/Events/SceneEvent.h"
#include "Modules/Particles/ParticleContext.h"
#include "Modules/Assets/Managers/SceneManager.h"

namespace Kargono::Scenes
{
	void SceneContext::Init()
	{
		// TODO: Previously had register has component func
	}

	void SceneContext::Terminate()
	{
		// Custom closing of input spec
		m_RenderSceneSpec.m_Shader.Reset();
		m_RenderSceneSpec.m_Texture.Reset();
		m_RenderSceneSpec.m_ShapeComponent = nullptr;
		m_RenderSceneSpec.m_CurrentDrawBuffer = nullptr;

		m_ActiveScene.Reset();
	}

	bool SceneContext::IsSceneActive(UUID sceneID)
	{
		if (!m_ActiveScene)
		{
			return false;
		}
		return m_ActiveScene.GetAssetHandle() == sceneID;
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
		Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(newSceneHandle, Events::ManageSceneAction::Open);
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);

	}

	void SceneContext::TransitionScene(Assets::AssetRef<Scene> newScene)
	{
		if (!newScene) { return; }

		Physics::Physics2DService().GetActiveContext().Terminate();
		Physics::Physics2DService().RemovePhysics2DWorld();
		m_ActiveScene->OnRuntimeStop();
		m_ActiveScene->m_EntityRegistry.ClearEntities();
		m_ActiveScene.Reset();

		m_ActiveScene = newScene;

		m_ActiveScene->ClearHoveredEntity();
		m_ActiveScene->ClearSelectedEntity();

		Physics::Physics2DService::CreatePhysics2DWorld();
		Physics::Physics2DService::GetActiveContext().Init
		(
			m_ActiveScene.GetAssetPtr(),
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

			Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(sceneID, Events::ManageSceneAction::Open);
			EngineService::GetActiveEngine().GetThread().SubmitEvent(event);

			Particles::ParticleService::GetActiveContext().LoadSceneEmitters(sceneReference);
		}
		else
		{
			KG_WARN("Attempt to transition scenes, however, new scene reference could not be found!");
		}
	}

	Assets::AssetRef<Scene> SceneContext::GetActiveScene()
	{
		return m_ActiveScene.GetAssetRef();
	}

	void SceneContext::ClearActiveScene()
	{
		m_ActiveScene.Reset();
	}

	void SceneContext::SetActiveScene(Assets::AssetRef<Scene> newScene)
	{
		KG_ASSERT(newScene.IsUsable());

		m_ActiveScene = newScene;

		Ref<Events::ManageScene> event = CreateRef<Events::ManageScene>(newScene.GetHandle(), Events::ManageSceneAction::Open);
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
	}
}