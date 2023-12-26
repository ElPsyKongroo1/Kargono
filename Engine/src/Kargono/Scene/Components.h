#pragma once

#include "Kargono/Core/UUID.h"
#include "Kargono/Scene/SceneCamera.h"
#include "Kargono/Renderer/Texture.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Renderer/Shape.h"
#include "Kargono/Audio/AudioEngine.h"
#include "Kargono/Math/Math.h"

#include <string>
#include <unordered_map>


namespace Kargono
{
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

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string tag)
			: Tag(tag) {}
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
		SceneCamera Camera;
		bool Primary = true; // TODO: Think about moving to scene
		//bool FixedAspectRatio = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct AudioComponent
	{
		std::string Name;
		Ref<Audio::AudioBuffer> Audio;
		Assets::AssetHandle AudioHandle;

		AudioComponent() = default;
		AudioComponent(const AudioComponent&) = default;
	};

	struct MultiAudioComponent
	{
		std::unordered_map<std::string, AudioComponent> AudioComponents{};

		MultiAudioComponent() = default;
		MultiAudioComponent(const MultiAudioComponent&) = default;
	};

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	// Physics
	struct Rigidbody2DComponent
	{
		enum class BodyType {Static = 0, Dynamic, Kinematic};
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// Storage for runtime
		void* RuntimeBody = nullptr;

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
		ShapeTypes CurrentShape = ShapeTypes::None;
		Ref<std::vector<Math::vec3>> Vertices {};
		Ref<std::vector<Math::vec2>> TextureCoordinates {};
		Ref<std::vector<uint32_t>> Indices {};
		Ref<std::vector<Math::vec4>> VertexColors {};
		Ref<Shader> Shader;
		ShaderSpecification ShaderSpecification {ColorInputType::None, TextureInputType::None, false, true, true, RenderingType::DrawIndex, false};
		Assets::AssetHandle ShaderHandle;
		Ref<Texture2D> Texture;
		Assets::AssetHandle TextureHandle;
		Buffer ShaderData;

		ShapeComponent()
		{
			auto [handle , shader] = Assets::AssetManager::GetShader(ShaderSpecification);
			ShaderHandle = handle;
			Shader = shader;
			Buffer textureBuffer{ 4 };
			textureBuffer.SetDataToByte(0xff);
			TextureHandle = Assets::AssetManager::ImportNewTextureFromData(textureBuffer, 1, 1, 4);
			Texture = Assets::AssetManager::GetTexture(TextureHandle);
			textureBuffer.Release();
			Buffer buffer(Shader->GetInputLayout().GetStride() * sizeof(uint8_t));
			ShaderData = buffer;
			ShaderData.SetDataToByte(0);
		}
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

	template<typename... Components>
	struct ComponentGroup
	{
	};

	using AllComponents = ComponentGroup<MultiAudioComponent, AudioComponent , TransformComponent, CameraComponent, ScriptComponent,
	Rigidbody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent, ShapeComponent,
	TagComponent>;
}
