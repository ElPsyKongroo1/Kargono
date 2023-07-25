#include <Kargono.h>

class ExampleLayer : public Kargono::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		m_VertexArray.reset(Kargono::VertexArray::Create());

		float vertices[3 * 7] =
		{
			//  X      Y      Z
				-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
		};
		std::shared_ptr<Kargono::VertexBuffer> vertexBuffer;
		vertexBuffer.reset(Kargono::VertexBuffer::Create(vertices, sizeof(vertices)));

		Kargono::BufferLayout layout = {
			{Kargono::ShaderDataType::Float3,  "a_Position"},
			{Kargono::ShaderDataType::Float4,  "a_Color"}
		};

		vertexBuffer->SetLayout(layout);
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		unsigned int indices[3] = { 0, 1, 2 };
		std::shared_ptr<Kargono::IndexBuffer> indexBuffer;
		indexBuffer.reset(Kargono::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_SquareVA.reset(Kargono::VertexArray::Create());

		float squareVertices[3 * 4] =
		{
			//  X      Y      Z
				-0.5f, -0.5f, 0.0f,
				0.5f, -0.5f, 0.0f,
				0.5f, 0.5f, 0.0f,
				-0.5, 0.5f, 0.0f
		};

		std::shared_ptr<Kargono::VertexBuffer> squareVB(Kargono::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{Kargono::ShaderDataType::Float3,  "a_Position"}
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		std::shared_ptr<Kargono::IndexBuffer> squareIB(Kargono::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string vertexSrc = R"(
			#version 450 core
		
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;
	
			void main ()
			{
				v_Color = a_Color;
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
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


		m_Shader.reset(new Kargono::Shader(vertexSrc, fragmentSrc));

		std::string blueShaderVertexSrc = R"(
			#version 450 core
		
			layout(location = 0) in vec3 a_Position;
			out vec3 v_Position;
	
			uniform mat4 u_ViewProjection;
			void main ()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
			}
		)";

		std::string blueShaderFragmentSrc = R"(
			#version 450 core
		
			layout(location = 0) out vec4 color;
			in vec3 v_Position;
			in vec4 v_Color;
	
			void main ()
			{
				color = vec4(0.2f, 0.3f, 0.5f, 1.0f);
			}
		)";


		m_BlueShader.reset(new Kargono::Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
	}

	void OnUpdate() override
	{
		Kargono::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		Kargono::RenderCommand::Clear();


		m_Camera.SetRotation(45.0f);

		Kargono::Renderer::BeginScene(m_Camera);

		Kargono::Renderer::Submit(m_BlueShader, m_SquareVA);

		Kargono::Renderer::Submit(m_Shader, m_VertexArray);

		Kargono::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		
	}

	void OnEvent(Kargono::Event& event) override 
	{
		Kargono::EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Kargono::KeyPressedEvent>(KG_BIND_EVENT_FN(ExampleLayer::OnKeyPressedEvent));
	}

	bool OnKeyPressedEvent(Kargono::KeyPressedEvent& event)
	{
		if (event.GetKeyCode() == KG_KEY_LEFT)
		{
			
		}
		return false;
	}
private:
	std::shared_ptr<Kargono::Shader> m_Shader;
	std::shared_ptr<Kargono::Shader> m_BlueShader;
	std::shared_ptr<Kargono::VertexArray> m_VertexArray;

	Kargono::OrthographicCamera m_Camera;


	std::shared_ptr<Kargono::VertexArray> m_SquareVA;
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