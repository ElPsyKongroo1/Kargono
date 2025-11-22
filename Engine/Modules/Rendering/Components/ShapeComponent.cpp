#include "kgpch.h"

#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/Assets/Managers/Texture2DManager.h"
#include "Modules/Assets/Managers/ShaderManager.h"

#include "Kargono/Core/Buffer.h"

namespace Kargono::Rendering
{
	void ShapeComponent::Serialize(void* context)
	{
		// Get context
		ECSInternal::SerializeComponentContext* serializeContext =
			(ECSInternal::SerializeComponentContext*)context;
		KG_ASSERT(serializeContext, "Context cannot be null");
		KG_ASSERT(serializeContext->m_Serializer, "Serializer cannot be null");
		// Get serializer
		YAML::Emitter& out = *serializeContext->m_Serializer;
		// Serialize component
		out << YAML::Key << "ShapeComponent";
		out << YAML::BeginMap; // Component Map
		out << YAML::Key << "CurrentShape" << YAML::Value << Utility::ShapeTypeToString(m_CurrentShape);
		if (m_VertexColors)
		{
			out << YAML::Key << "VertexColors" << YAML::Value << YAML::BeginSeq;
			for (const auto& color : *m_VertexColors)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Color" << YAML::Value << color;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
		}
		if (m_Texture)
		{
			out << YAML::Key << "TextureHandle" << YAML::Value << static_cast<uint64_t>(m_Texture.GetAssetHandle());
		}

		static_assert(sizeof(uint8_t) * 20 == sizeof(Rendering::ShaderSpecification));
		if (m_Shader)
		{
			// Add Shader Handle
			out << YAML::Key << "ShaderHandle" << YAML::Value << static_cast<uint64_t>(m_Shader.GetAssetHandle());
			// Add Shader Specification
			const Rendering::ShaderSpecification& shaderSpec = m_Shader->GetSpecification();
			out << YAML::Key << "ShaderSpecification" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "ColorInputType" << YAML::Value << Utility::ColorInputTypeToString(shaderSpec.m_ColorInput);
			out << YAML::Key << "AddProjectionMatrix" << YAML::Value << shaderSpec.m_AddProjectionMatrix;
			out << YAML::Key << "AddEntityID" << YAML::Value << shaderSpec.m_AddEntityID;
			out << YAML::Key << "AddCircleShape" << YAML::Value << shaderSpec.m_AddCircleShape;
			out << YAML::Key << "TextureInput" << YAML::Value << Utility::TextureInputTypeToString(shaderSpec.m_TextureInput);
			out << YAML::Key << "DrawOutline" << YAML::Value << shaderSpec.m_DrawOutline;
			out << YAML::Key << "RenderType" << YAML::Value << Utility::RenderingTypeToString(shaderSpec.m_RenderType);

			out << YAML::EndMap;
			// Add Buffer
			out << YAML::Key << "Buffer" << YAML::Value << YAML::Binary(m_ShaderData.m_Data, m_ShaderData.m_Size);
		}
		out << YAML::EndMap; // Component Map
	}

	void ShapeComponent::Deserialize(void* context)
	{
		// Get context
		ECSInternal::DeserializeComponentContext* deserializeContext =
			(ECSInternal::DeserializeComponentContext*)context;
		KG_ASSERT(deserializeContext, "Context cannot be null");
		KG_ASSERT(deserializeContext->m_Node, "Registry node cannot be null");
		// Get node
		YAML::Node& node = *deserializeContext->m_Node;

		// Deserialize
		m_CurrentShape = Utility::StringToShapeType(node["CurrentShape"].as<std::string>());
		if (node["VertexColors"])
		{
			YAML::Node vertexColors = node["VertexColors"];
			m_VertexColors = CreateRef<std::vector<Math::vec4>>();
			for (const YAML::Node& color : vertexColors)
			{
				m_VertexColors->push_back(color["Color"].as<Math::vec4>());
			}
		}

		if (node["TextureHandle"])
		{
			Assets::AssetHandle textureHandle = node["TextureHandle"].as<uint64_t>();
			m_Texture = Assets::s_Texture2DManager.GetAssetByHandle(textureHandle);
		}

		if (node["ShaderHandle"])
		{
			Assets::AssetHandle shaderHandle = node["ShaderHandle"].as<uint64_t>();
			m_Shader = Assets::s_ShaderManager.GetAssetByHandle(shaderHandle);
			if (!m_Shader)
			{
				YAML::Node shaderSpecificationNode = node["ShaderSpecification"];
				Rendering::ShaderSpecification shaderSpec{};
				// ShaderSpecification Section
				shaderSpec.m_ColorInput = Utility::StringToColorInputType(shaderSpecificationNode["ColorInputType"].as<std::string>());
				shaderSpec.m_AddProjectionMatrix = shaderSpecificationNode["AddProjectionMatrix"].as<bool>();
				shaderSpec.m_AddEntityID = shaderSpecificationNode["AddEntityID"].as<bool>();
				shaderSpec.m_AddCircleShape = shaderSpecificationNode["AddCircleShape"].as<bool>();
				shaderSpec.m_TextureInput = Utility::StringToTextureInputType(shaderSpecificationNode["TextureInput"].as<std::string>());
				shaderSpec.m_DrawOutline = shaderSpecificationNode["DrawOutline"].as<bool>();
				shaderSpec.m_RenderType = Utility::StringToRenderingType(shaderSpecificationNode["RenderType"].as<std::string>());
				Assets::AssetRef<Shader> newShader = Assets::s_ShaderManager.GetAssetBySpec(shaderSpec);
				shaderHandle = newShader.GetHandle();
				m_Shader = newShader;
			}
			m_ShaderSpecification = m_Shader->GetSpecification();
			YAML::Binary binary = node["Buffer"].as<YAML::Binary>();
			Buffer buffer{ binary.size() };
			memcpy(buffer.m_Data, binary.data(), buffer.m_Size);
			m_ShaderData = buffer;
			if (m_CurrentShape != Rendering::ShapeTypes::None)
			{
				if (m_ShaderSpecification.m_RenderType == Rendering::RenderingType::DrawIndex)
				{
					m_Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(m_CurrentShape).GetIndexVertices());
					m_Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(m_CurrentShape).GetIndices());
					m_TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(m_CurrentShape).GetIndexTextureCoordinates());
				}

				if (m_ShaderSpecification.m_RenderType == Rendering::RenderingType::DrawTriangle)
				{
					m_Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(m_CurrentShape).GetTriangleVertices());
					m_TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(m_CurrentShape).GetTriangleTextureCoordinates());
				}
			}
		}
	}

	ShapeComponent::ShapeComponent()
	{
		Assets::AssetRef<Shader> shader = Assets::s_ShaderManager.GetAssetBySpec(m_ShaderSpecification);
		m_Shader = shader;
		Buffer textureBuffer{ 4 };
		textureBuffer.SetDataToByte(0xff);
		Rendering::TextureSpecification textureSpec{};
		textureSpec.m_Buffer = textureBuffer;
		textureSpec.m_Width = 1;
		textureSpec.m_Height = 1;
		textureSpec.m_Format = ImageFormat::RGBA8;
		Assets::AssetRef<Texture2D> texture = Assets::s_Texture2DManager.CreateAssetFromSpec({}, textureSpec);
		KG_ASSERT(texture.IsUsable());
		m_Texture = texture;
		textureBuffer.Release();
		Buffer buffer(m_Shader->GetInputLayout().GetStride() * sizeof(uint8_t));
		m_ShaderData = buffer;
		m_ShaderData.SetDataToByte(0);
	}
	ShapeComponent::~ShapeComponent()
	{
		if (m_ShaderData)
		{
			m_ShaderData.Release();
		}
	}
}