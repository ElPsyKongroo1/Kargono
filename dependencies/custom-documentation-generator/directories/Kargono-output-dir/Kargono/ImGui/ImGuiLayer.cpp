/// @brief Include the precompiled header file for faster compilation time.
#include "Kargono/kgpch.h"
/// @brief Include the header file for the ImGui layer.
#include "ImGuiLayer.h"

/// @brief Include the ImGui library.
#include "imgui.h"
/// @brief Include the ImGui OpenGL implementation.
#include "Platform/OpenGL/imgui_impl_opengl3.h"
/// @brief Include the GLFW library.
#include "GLFW/glfw3.h"
/// @brief Include the application header file.
#include "Kargono/Application.h"

/// @brief Define the namespace for the engine.
namespace Kargono 
{
/// @brief Define the constructor of the ImGuiLayer class.
	ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer")
	{
	
	}
/// @brief Define the destructor of the ImGuiLayer class.
	ImGuiLayer::~ImGuiLayer() 
	{

	}

/// @brief Called when the ImGuiLayer is attached to the layer stack.
	void ImGuiLayer::OnAttach()
	{
/// @brief Create the ImGui context.
		ImGui::CreateContext();
/// @brief Apply the "Dark" theme to ImGui.
		ImGui::StyleColorsDark();

/// @brief Get the ImGuiIO instance.
		ImGuiIO& io = ImGui::GetIO();
/// @brief Set the ImGui backend flags for mouse cursors.
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
/// @brief Set the ImGui backend flags for setting mouse position.
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		// TEMPORARY FIXME
/// @brief Map the ImGui key 'Tab' to the GLFW key 'Tab'.
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
/// @brief Map the ImGui key 'LeftArrow' to the GLFW key 'Left'.
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
/// @brief Map the ImGui key 'RightArrow' to the GLFW key 'Right'.
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
/// @brief Map the ImGui key 'UpArrow' to the GLFW key 'Up'.
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
/// @brief Map the ImGui key 'DownArrow' to the GLFW key 'Down'.
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
/// @brief Map the ImGui key 'PageUp' to the GLFW key 'PageUp'.
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
/// @brief Map the ImGui key 'PageDown' to the GLFW key 'PageDown'.
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
/// @brief Map the ImGui key 'Home' to the GLFW key 'Home'.
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
/// @brief Map the ImGui key 'End' to the GLFW key 'End'.
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
/// @brief Map the ImGui key 'Insert' to the GLFW key 'Insert'.
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
/// @brief Map the ImGui key 'Delete' to the GLFW key 'Delete'.
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
/// @brief Map the ImGui key 'Backspace' to the GLFW key 'Backspace'.
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
/// @brief Map the ImGui key 'Space' to the GLFW key 'Space'.
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
/// @brief Map the ImGui key 'Enter' to the GLFW key 'Enter'.
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
/// @brief Map the ImGui key 'Escape' to the GLFW key 'Escape'.
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
/// @brief Map the ImGui key 'A' to the GLFW key 'A'.
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
/// @brief Map the ImGui key 'C' to the GLFW key 'C'.
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
/// @brief Map the ImGui key 'V' to the GLFW key 'V'.
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
/// @brief Map the ImGui key 'X' to the GLFW key 'X'.
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
/// @brief Map the ImGui key 'Y' to the GLFW key 'Y'.
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
/// @brief Map the ImGui key 'Z' to the GLFW key 'Z'.
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

/// @brief Initialize the ImGui OpenGL implementation.
		ImGui_ImplOpenGL3_Init("#version 410");
	}

/// @brief Called when the ImGuiLayer is detached from the layer stack.
/// @brief Called when the ImGuiLayer is detached from the layer stack.
	void ImGuiLayer::OnDetach()
	{
	}

/// @brief Update the ImGui layer.
/// @brief Update the ImGui layer.
	void ImGuiLayer::OnUpdate()
	{
		
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

/// @brief Get the current time using GLFW.
		float time = (float)glfwGetTime();
/// @brief Calculate the time elapsed since the last frame.
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
/// @brief Update the previous frame time.
		m_Time = time;

/// @brief Start a new ImGui frame for OpenGL rendering.
		ImGui_ImplOpenGL3_NewFrame();
/// @brief Start a new ImGui frame.
		ImGui::NewFrame();

/// @brief Initialize a static boolean variable.
		static bool show = true;
/// @brief Show the ImGui demo window.
		ImGui::ShowDemoWindow(&show);

/// @brief Render the ImGui frame.
		ImGui::Render();
/// @brief Render the ImGui draw data for OpenGL.
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
/// @brief Process events on the ImGui layer.
	void ImGuiLayer::OnEvent(Event& event) 
	{

	}
	
}