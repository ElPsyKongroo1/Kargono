#pragma once
#include "Kargono.h"

class Sandbox2D : public Kargono::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;
	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnUpdate(Kargono::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Kargono::Event& event) override;
private:

	Kargono::OrthographicCameraController m_CameraController;

	// Temp
	Kargono::Ref<Kargono::Shader> m_FlatColorShader;
	Kargono::Ref<Kargono::VertexArray> m_SquareVA;

	Kargono::Ref<Kargono::Texture2D> m_CheckerboardTexture;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};