#include <Kargono.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "dependencies/imgui/imgui.h"
#include "Platform/OpenGL/OpenGLShader.h"

class ExampleLayer : public Kargono::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_CameraController(1280.0f / 720.0f, true)
	{
		m_VertexArray.reset(Kargono::VertexArray::Create());

		float vertices[3 * 7] =
		{
			//  X      Y      Z
				-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
		};
		Kargono::Ref<Kargono::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Kargono::VertexBuffer::Create(vertices, sizeof(vertices)));

		Kargono::BufferLayout layout = {
			{Kargono::ShaderDataType::Float3,  "a_Position"},
			{Kargono::ShaderDataType::Float4,  "a_Color"}
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[3] = { 0, 1, 2 };
		Kargono::Ref<Kargono::IndexBuffer> indexBuffer;
		indexBuffer.reset(Kargono::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(Kargono::VertexArray::Create());

		float squareVertices[5 * 4] =
		{
			//  X      Y      Z
				-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
				 0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
				-0.5, 0.5f, 0.0f, 0.0f, 1.0f
		};

		Kargono::Ref<Kargono::VertexBuffer> squareVB(Kargono::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{Kargono::ShaderDataType::Float3,  "a_Position"},
		 {Kargono::ShaderDataType::Float2,  "a_TexCoord"}
				
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Kargono::Ref<Kargono::IndexBuffer> squareIB(Kargono::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 450 core
		
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;
	
			void main ()
			{
				v_Color = a_Color;
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
			}
		)";

		std::string fragmentSrc = R"(
			#version 450 core
		
			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			in vec4 v_Color;
	
			void main ()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0f);
				color = v_Color;
			}
		)";


		m_Shader = Kargono::Shader::Create("VertexColorTriangle", vertexSrc, fragmentSrc);

		//Shader::Create("assets/shaders/Texture.glsl");

		std::string flatColorShaderVertexSrc = R"(
			#version 450 core
		
			layout(location = 0) in vec3 a_Position;
			out vec3 v_Position;
	
			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;
			void main ()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
			}
		)";

		std::string flatColorShaderFragmentSrc = R"(
			#version 450 core
		
			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			
			uniform vec3 u_Color;
			
	
			void main ()
			{
				color = vec4(u_Color, 1.0f);
			}
		)";

		m_FlatColorShader = Kargono::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = Kargono::Texture2D::Create("assets/textures/Checkerboard.png");
		m_ChernoLogoTexture = Kargono::Texture2D::Create("assets/textures/ChernoLogo.png");

		std::dynamic_pointer_cast<Kargono::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Kargono::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);

	}

	void OnUpdate(Kargono::Timestep ts) override
	{
		// Update
		m_CameraController.OnUpdate(ts);

		Kargono::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Kargono::RenderCommand::Clear();

		Kargono::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Kargono::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Kargono::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color",m_SquareColor);
		for (int y{ 0 }; y < 20; y++)
		{
			for (int x{ 0 }; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Kargono::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Kargono::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		m_ChernoLogoTexture->Bind();
		Kargono::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		// Triangle
		//Kargono::Renderer::Submit(m_Shader, m_VertexArray);

		Kargono::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Kargono::Event& e) override 
	{
		m_CameraController.OnEvent(e);
	}

private:
	Kargono::ShaderLibrary m_ShaderLibrary;
	Kargono::Ref<Kargono::Shader> m_Shader;
	Kargono::Ref<Kargono::Shader> m_FlatColorShader;
	Kargono::Ref<Kargono::VertexArray> m_VertexArray;
	Kargono::Ref<Kargono::VertexArray> m_SquareVA;
	Kargono::Ref<Kargono::Texture2D> m_Texture, m_ChernoLogoTexture;

	Kargono::OrthographicCameraController m_CameraController;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

};

class Sandbox2D : public Kargono::Application 
{
public:
	Sandbox2D() 
	{
		PushLayer(new ExampleLayer()); 
	}
	~Sandbox2D() 
	{
		
	}
};

Kargono::Application* Kargono::CreateApplication()
{
	return new Sandbox2D();
}