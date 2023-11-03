#include "kgpch.h"

#include "Kargono/Scene/Scene.h"
#include "Kargono/Scene/Components.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Scripting/ScriptEngine.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Renderer/Shader.h"

#include <glm/glm.hpp>



namespace Kargono
{

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;
		newScene->m_PhysicsSpecification = other->m_PhysicsSpecification;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
		
	}
	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		m_EntityMap[uuid] = entity;
		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());
		if (m_Registry.valid(entity))
		{
			m_Registry.destroy(entity);
		}
	}

	void Scene::DestroyAllEntities()
	{
		if (m_EntityMap.empty()) { return; }
		for (auto& [uuid, entity] : m_EntityMap)
		{
			if (m_Registry.valid(entity))
			{
				m_Registry.destroy(entity);
			}
		}
		m_EntityMap.clear();
	}
	
	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;
		// Physics
		m_PhysicsWorld = CreateScope<Physics2DWorld>(this, m_PhysicsSpecification.Gravity);

		// Scripts
		ScriptEngine::OnRuntimeStart(this);
	}

	void Scene::OnRuntimeStop()
	{
		m_IsRunning = false;
		m_PhysicsWorld.reset();
		m_PhysicsWorld = nullptr;

		// Script
		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnSimulationStart()
	{
		// Physics
		m_PhysicsWorld = CreateScope<Physics2DWorld>(this, m_PhysicsSpecification.Gravity);
	}

	void Scene::OnSimulationStop()
	{
		// Physics
		m_PhysicsWorld.reset();
		m_PhysicsWorld = nullptr;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		// Copy name because we're going to modify component data structure
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name) { return Entity{ entity, this }; }
		}
		return {};
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		// TODO: Maybe should be an assert
		if (m_EntityMap.find(uuid) != m_EntityMap.end()) { return { m_EntityMap.at(uuid), this }; }
		return {};
	}

	bool Scene::CheckEntityExists(entt::entity entity)
	{
		return m_Registry.valid(entity);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (m_ViewportWidth == width && m_ViewportHeight == height) { return; }

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize non-fixed
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}

	}
	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity: view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary) { return Entity{ entity, this }; }
		}
		return {};
	}
	void Scene::RenderScene(Camera& camera, const glm::mat4& transform)
	{
		Renderer::BeginScene(camera, transform);
		// Draw Shapes
		{
			auto view = m_Registry.view<TransformComponent, ShapeComponent>();
			for (auto entity : view)
			{
				const auto& [transform, shape] = view.get<TransformComponent, ShapeComponent>(entity);

				static Shader::RendererInputSpec inputSpec{};
				inputSpec.Shader = shape.Shader;
				inputSpec.Buffer = shape.ShaderData;
				inputSpec.Entity = static_cast<uint32_t>(entity);
				inputSpec.EntityRegistry = &m_Registry;
				inputSpec.ShapeComponent = &shape;
				inputSpec.TransformMatrix = transform.GetTransform();

				for (const auto& PerObjectSceneFunction : shape.Shader->GetFillDataObjectScene())
				{
					PerObjectSceneFunction(inputSpec);
				}

				Renderer::SubmitDataToRenderer(inputSpec);
			}
		}

		Renderer::EndScene();

	}
	void Scene::OnUpdatePhysics(Timestep ts)
	{
		m_PhysicsWorld->OnUpdate(ts);
	}
	void Scene::FillEntityID(Shader::RendererInputSpec& inputSpec)
	{
		Shader::SetDataAtInputLocation<uint32_t>(inputSpec.Entity, "a_EntityID", inputSpec.Buffer, inputSpec.Shader);
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
}
