#include "Kargono/kgpch.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Scene/Components.h"
#include <glm/glm.hpp>

namespace Kargono 
{
	static void DoMath(const glm::mat4& transform)
	{
		
	}

	Scene::Scene()
	{
		struct MeshComponent
		{
			float Value;
			MeshComponent() = default;
		};

		struct TransformComponent
		{
			glm::mat4 Transform;

			TransformComponent() = default;
			TransformComponent(const TransformComponent&) = default;
			TransformComponent(const glm::mat4& transform)
				: Transform(transform) {}

			operator glm::mat4& () { return Transform; }
			operator const glm::mat4& () const { return Transform; }
		};

		entt::entity entity = m_Registry.create();

		m_Registry.emplace<TransformComponent>(entity, glm::mat4(1.0f));

		auto view = m_Registry.view<TransformComponent>();
		for (auto entity: view)
		{
			auto transform = view.get<TransformComponent>(entity);
		}

	}

	Scene::~Scene()
	{
	}
	entt::entity Scene::CreateEntity()
	{
		return m_Registry.create();
	}

	void Scene::OnUpdate(Timestep ts)
	{
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group)
		{
			const auto& [transform, mesh] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			Renderer2D::DrawQuad()
		}

	}

}
