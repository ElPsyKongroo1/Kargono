#pragma once

#include "Kargono/Core/UUID.h"
#include "Kargono/Scenes/SceneCamera.h"
#include "Kargono/Rendering/Shader.h"
#include "Kargono/Rendering/Shape.h"
#include "Kargono/Audio/Audio.h"
#include "Kargono/Math/Math.h"
#include "Kargono/AI/AIService.h"

#include <string>
#include <unordered_map>

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::ECS
{
	struct AIStateComponent
	{
		// Main state that undergoes state transitions
		Assets::AssetHandle CurrentStateHandle{ Assets::EmptyHandle };
		Ref<AI::AIState> CurrentStateReference{ nullptr };
		// This state allows transitioning into a temporary state for the CurrentState and easily reverting back
		Assets::AssetHandle PreviousStateHandle{ Assets::EmptyHandle };
		Ref<AI::AIState> PreviousStateReference{ nullptr };
		// Globally held state that is ubiquitously available and ran in the OnUpdate() function
		Assets::AssetHandle GlobalStateHandle{ Assets::EmptyHandle };
		Ref<AI::AIState> GlobalStateReference{ nullptr };

		AIStateComponent() = default;
		AIStateComponent(const AIStateComponent&) = default;
	};

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid)
			: ID(uuid) {}
	};

	struct TagComponent
	{
		std::string Tag;
		std::string Group;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string tag)
			: Tag(tag) {}
		TagComponent(const std::string tag, const std::string& group)
			: Tag(tag), Group(group) {}
	};

	struct TransformComponent
	{
		Math::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		Math::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		Math::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Math::vec3& translation)
			: Translation(translation) {}

		Math::mat4 GetTransform() const
		{
			Math::mat4 rotation = glm::toMat4(Math::quat(Rotation));

			return glm::translate(Math::mat4(1.0f), Translation)
				* rotation
				* glm::scale(Math::mat4(1.0f), Scale);
		}

		Math::mat4 GetTranslation() const
		{
			return glm::translate(Math::mat4(1.0f), Translation);
		}

		Math::mat4 GetRotation() const
		{
			return glm::toMat4(Math::quat(Rotation));
		}
	};

	struct CameraComponent
	{
		Scenes::SceneCamera Camera;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	enum class NetworkType
	{
		None = 0,
		ClientAuthoritative = 1,
		ServerTime = 2,
		TrustLastCollision = 3
	};

	struct NetworkComponent
	{
		bool NetworkPhysics = false;
		NetworkType PhysicsType = NetworkType::None;
	};

	struct OnCreateComponent
	{
		Assets::AssetHandle OnCreateScriptHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> OnCreateScript{ nullptr };
	};

	struct OnUpdateComponent
	{
		Assets::AssetHandle OnUpdateScriptHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> OnUpdateScript{ nullptr };
	};

	// Physics
	struct Rigidbody2DComponent 
	{
		enum class BodyType
		{
			Static = 0,
			Dynamic,
			Kinematic
		};

		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		// Collision callback function pointers
		Assets::AssetHandle OnCollisionStartScriptHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> OnCollisionStartScript{ nullptr };

		Assets::AssetHandle OnCollisionEndScriptHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> OnCollisionEndScript{ nullptr };

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		Math::vec2 Offset = { 0.0f, 0.0f };
		Math::vec2 Size = { 0.5f, 0.5f };

		// TODO: move into physics material maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};


	struct CircleCollider2DComponent
	{
		Math::vec2 Offset = { 0.0f, 0.0f };
		float Radius =  0.5f;

		// TODO: move into physics material maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	struct ShapeComponent
	{
		Rendering::ShapeTypes CurrentShape = Rendering::ShapeTypes::None;
		Ref<std::vector<Math::vec3>> Vertices {};
		Ref<std::vector<Math::vec2>> TextureCoordinates {};
		Ref<std::vector<uint32_t>> Indices {};
		Ref<std::vector<Math::vec4>> VertexColors {};
		Ref<Rendering::Shader> Shader;
		Rendering::ShaderSpecification ShaderSpecification {Rendering::ColorInputType::None, Rendering::TextureInputType::None, false, true, true, Rendering::RenderingType::DrawIndex, false};
		Assets::AssetHandle ShaderHandle;
		Ref<Rendering::Texture2D> Texture;
		Assets::AssetHandle TextureHandle;
		Buffer ShaderData;

		ShapeComponent();
		ShapeComponent(const ShapeComponent& other)
		{
			this->CurrentShape = other.CurrentShape;
			this->Vertices = other.Vertices;
			this->TextureCoordinates = other.TextureCoordinates;
			this->Indices = other.Indices;
			this->Shader = other.Shader;
			this->ShaderSpecification = other.ShaderSpecification;
			this->Texture = other.Texture;
			this->TextureHandle = other.TextureHandle;
			this->ShaderHandle = other.ShaderHandle;
			this->ShaderData = Buffer::Copy(other.ShaderData);
			this->VertexColors = other.VertexColors;
			//->ShaderData = other.ShaderData;
		}

		ShapeComponent& operator=(const ShapeComponent& other)
		{

			this->CurrentShape = other.CurrentShape;
			this->Vertices = other.Vertices;
			this->TextureCoordinates = other.TextureCoordinates;
			this->Indices = other.Indices;
			this->Shader = other.Shader;
			this->ShaderSpecification = other.ShaderSpecification;
			this->Texture = other.Texture;
			this->TextureHandle = other.TextureHandle;
			this->ShaderHandle = other.ShaderHandle;
			this->ShaderData = Buffer::Copy(other.ShaderData);
			this->VertexColors = other.VertexColors;
			//->ShaderData = other.ShaderData;
			return *this;
		}
		~ShapeComponent()
		{
			if (ShaderData)
			{
				ShaderData.Release();
				//KG_CORE_INFO("Shape Destructor Called");
			}
			
		}
	};

	enum class ComponentType : uint16_t
	{
		None = 0,
		Transform,
		Tag,
		Camera,
		Rigidbody2D,
		BoxCollider2D,
		CircleCollider2D,
		OnUpdate,
		OnCreate,
		Shape,
		Network,
		AIState,
		ProjectComponent
	};

	template<typename... Components>
	struct ComponentGroup
	{
	};

	using AllComponents = ComponentGroup<TransformComponent, CameraComponent, AIStateComponent,
	Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent, ShapeComponent,
	TagComponent, OnUpdateComponent, OnCreateComponent, NetworkComponent>;
}
