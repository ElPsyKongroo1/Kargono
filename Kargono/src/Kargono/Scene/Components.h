#pragma once

#include "Kargono/Core/UUID.h"
#include "Kargono/Scene/SceneCamera.h"
#include "Kargono/Renderer/Texture.h"
#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Renderer/Shader.h"
#include "Kargono/Renderer/Shape.h"
#include "Kargono/Audio/AudioEngine.h"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

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
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}

		glm::mat4 GetTranslation() const
		{
			return glm::translate(glm::mat4(1.0f), Translation);
		}

		glm::mat4 GetRotation() const
		{
			return glm::toMat4(glm::quat(Rotation));
		}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true; // TODO: Think about moving to scene
		bool FixedAspectRatio = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct AudioComponent
	{
		std::string Name;
		Ref<AudioBuffer> Audio;
		AssetHandle AudioHandle;

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
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

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
		glm::vec2 Offset = { 0.0f, 0.0f };
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
		Shape::ShapeTypes CurrentShape = Shape::ShapeTypes::None;
		Ref<std::vector<glm::vec3>> Vertices {};
		Ref<std::vector<glm::vec2>> TextureCoordinates {};
		Ref<std::vector<uint32_t>> Indices {};
		Ref<std::vector<glm::vec4>> VertexColors {};
		Ref<Shader> Shader;
		Shader::ShaderSpecification ShaderSpecification {Shader::ColorInputType::None, Shader::TextureInputType::None, false, true, true, Shape::RenderingType::DrawIndex, false};
		AssetHandle ShaderHandle;
		Ref<Texture2D> Texture;
		AssetHandle TextureHandle;
		Buffer ShaderData;

		ShapeComponent()
		{
			auto [handle , shader] = AssetManager::GetShader(ShaderSpecification);
			ShaderHandle = handle;
			Shader = shader;
			Buffer textureBuffer{ 4 };
			textureBuffer.SetDataToByte(0xff);
			TextureHandle = AssetManager::ImportNewTextureFromData(textureBuffer, 1, 1, 4);
			Texture = AssetManager::GetTexture(TextureHandle);
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
