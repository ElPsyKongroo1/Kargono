#include "kgpch.h"

#include "Modules/RuntimeUI/FontContext.h"
#include "Modules/Rendering/Components/ShapeComponent.h"
#include "Modules/RuntimeUI/ExternalAPI/msdfgenAPI.h"
#include "Modules/Assets/Managers/ShaderManager.h"

namespace Kargono::RuntimeUI
{
	void FontContext::Init()
	{
		// TODO: Change to Proper Boolean Check
		if (!m_TextInputSpec.m_Shader)
		{
			// TODO: Unreleased Heap Data with Buffer
			Rendering::ShaderSpecification textShaderSpec{ Rendering::ColorInputType::FlatColor, Rendering::TextureInputType::TextTexture, false, true, true, Rendering::RenderingType::DrawTriangle, false };
			Assets::AssetRef<Rendering::Shader> localShader = Assets::s_ShaderManager.GetAssetBySpec(textShaderSpec);
			Buffer localBuffer{ localShader->GetInputLayout().GetStride() };

			Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 0.0f, 1.0f, 0.0f, 1.0f },
				Utility::FileSystem::CRCFromString("a_Color"),
				localBuffer, localShader);

			m_TextInputSpec.m_ShapeComponent = new Rendering::ShapeComponent();
			m_TextInputSpec.m_ShapeComponent->m_CurrentShape = Rendering::ShapeTypes::Quad;

			m_TextInputSpec.m_Shader = localShader;
			m_TextInputSpec.m_ShapeComponent->m_Shader = localShader;
			m_TextInputSpec.m_Buffer = localBuffer;
			m_TexCoordinates = CreateRef<std::vector<Math::vec2>>();
			m_TexCoordinates->push_back({ 0.0f, 0.0f });
			m_TexCoordinates->push_back({ 0.0f, 1.0f });
			m_TexCoordinates->push_back({ 1.0f, 1.0f });
			m_TexCoordinates->push_back({ 0.0f, 0.0f });
			m_TexCoordinates->push_back({ 1.0f, 1.0f });
			m_TexCoordinates->push_back({ 1.0f, 0.0f });
			m_TextInputSpec.m_ShapeComponent->m_TextureCoordinates = m_TexCoordinates;

			m_Vertices = CreateRef<std::vector<Math::vec3>>();
		}
		KG_VERIFY(true, "Runtime Text Engine Init")
	}

	void FontContext::Terminate()
	{
		m_TextInputSpec.ClearData();
	}

	void FontContext::SetID(uint32_t id)
	{
		Rendering::Shader::SetDataAtInputLocation<uint32_t>(id,
			Utility::FileSystem::CRCFromString("a_EntityID"),
			m_TextInputSpec.m_Buffer, m_TextInputSpec.m_Shader.GetAssetRef());
	}
}