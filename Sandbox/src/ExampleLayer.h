#pragma once

#include "Kargono.h"

class ExampleLayer : public Kargono::Layer
{
public:
	ExampleLayer();
	virtual ~ExampleLayer() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(Kargono::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Kargono::Event& e) override;
private:
	Kargono::ShaderLibrary m_ShaderLibrary;
	Kargono::Ref<Kargono::Shader> m_Shader;
	Kargono::Ref<Kargono::VertexArray> m_VertexArray;

	Kargono::Ref<Kargono::Shader> m_FlatColorShader;
	Kargono::Ref<Kargono::VertexArray> m_SquareVA;

	Kargono::Ref<Kargono::Texture2D> m_Texture, m_ChernoLogoTexture;

	Kargono::OrthographicCameraController m_CameraController;
	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};