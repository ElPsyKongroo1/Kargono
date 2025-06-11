#pragma once

#include "Kargono/Core/Base.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiAPI.h"

#include <filesystem>

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::EditorUI
{
	// Constant Colors
	constexpr ImVec4 k_PureWhite{ 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr ImVec4 k_PureEmpty{ 0.0f, 0.0f, 0.0f, 0.0f };
	constexpr ImVec4 k_PureBlack{ 0.0f, 0.0f, 0.0f, 1.0f };

	constexpr ImVec4 k_Red{ 255.0f / 255.0f, 40.0f / 255.0f, 73.0f / 255.0f, 1.0f };
	constexpr ImVec4 k_Green{ 147.0f / 255.0f, 247.0f / 255.0f, 141.4f / 255.0f, 1.0f };
	constexpr ImVec4 k_Blue{ 141.0f / 255.0f, 200.0f / 255.0f, 247.0f / 255.0f, 1.0f };
	constexpr ImVec4 k_Alpha{ 247.6f / 255.0f, 188.2f / 255.0f, 140.7f / 255.0f, 1.0f };

	struct ConfigColors
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		ConfigColors() = default;
		~ConfigColors() = default;
	public:
		//==============================
		// Public Fields
		//==============================
		// Text Color(s)
		ImVec4 s_PrimaryTextColor{ 242.0f / 255.0f, 236.5f / 255.0f, 221.1f / 255.0f, 1.0f };
		ImVec4 s_SecondaryTextColor{ 220.0f / 255.0f, 215.0f / 255.0f, 201.0f / 255.0f, 1.0f };
		// Interaction Color(s)
		ImVec4 s_HoveredColor{ 91.4f / 255.0f, 113.3f / 255.0f, 114.7f / 255.0f, 1.0f };
		ImVec4 s_ActiveColor{ 76.2f / 255.0f, 94.4f / 255.0f, 95.6f / 255.0f, 1.0f };
		ImVec4 s_SelectedColor{ 162.0f / 255.0f, 123.0f / 255.0f, 92.0f / 255.0f, 1.0f };
		ImVec4 s_DisabledColor{ 0.5f, 0.5f, 0.5f, 1.0f };
		// Background Color(s)
		ImVec4 s_BackgroundColor{ 44.0f / 255.0f, 54.0f / 255.0f, 57.0f / 255.0f, 1.0f };
		ImVec4 s_DarkBackgroundColor{ 35.2f / 255.0f, 43.2f / 255.0f, 45.6f / 255.0f, 1.0f };
		ImVec4 s_AccentColor{ 63.0f / 255.0f, 78.0f / 255.0f, 79.0f / 255.0f, 1.0f };
		ImVec4 s_DarkAccentColor{ 50.4f / 255.0f, 62.4f / 255.0f, 63.2f / 255.0f, 1.0f };
		// Highlight Color(s)
		ImVec4 s_HighlightColor1{ 247.6f / 255.0f, 188.2f / 255.0f, 140.7f / 255.0f, 1.0f };
		ImVec4 s_HighlightColor1_Thin{ s_HighlightColor1.x, s_HighlightColor1.y, s_HighlightColor1.z, s_HighlightColor1.w * 0.75f };
		ImVec4 s_HighlightColor1_UltraThin{ s_HighlightColor1.x, s_HighlightColor1.y, s_HighlightColor1.z, s_HighlightColor1.w * 0.3f };
		ImVec4 s_HighlightColor2{ 147.0f / 255.0f, 247.0f / 255.0f, 141.4f / 255.0f, 1.0f };
		ImVec4 s_HighlightColor2_Thin{ s_HighlightColor2.x, s_HighlightColor2.y, s_HighlightColor2.z, s_HighlightColor2.w * 0.75f };
		ImVec4 s_HighlightColor2_UltraThin{ s_HighlightColor2.x, s_HighlightColor2.y, s_HighlightColor2.z, s_HighlightColor2.w * 0.3f };
		ImVec4 s_HighlightColor3{ 241.0f / 255.0f, 141.0f / 255.0f, 247.4f / 255.0f, 1.0f };
		ImVec4 s_HighlightColor3_Thin{ s_HighlightColor3.x, s_HighlightColor3.y, s_HighlightColor3.z, s_HighlightColor3.w * 0.75f };
		ImVec4 s_HighlightColor3_UltraThin{ s_HighlightColor3.x, s_HighlightColor3.y, s_HighlightColor3.z, s_HighlightColor3.w * 0.3f };
		ImVec4 s_HighlightColor4{ 141.0f / 255.0f, 200.0f / 255.0f, 247.0f / 255.0f, 1.0f };
		ImVec4 s_HighlightColor4_Thin{ s_HighlightColor4.x, s_HighlightColor4.y, s_HighlightColor4.z, s_HighlightColor4.w * 0.75f };
		ImVec4 s_HighlightColor4_UltraThin{ s_HighlightColor4.x, s_HighlightColor4.y, s_HighlightColor4.z, s_HighlightColor4.w * 0.3f };
		// Grid Color(s)
		ImVec4 s_GridMajor{ 0.735f, 0.720f, 0.690f, 1.0f };
		ImVec4 s_GridMinor{ 0.347f, 0.347f, 0.347f, 1.0f };
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIService;
	};


	struct GeneralIcons
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		GeneralIcons() = default;
		~GeneralIcons() = default;
	public:
		//==============================
		// Create Icons
		//==============================
		void LoadIcons(const std::filesystem::path& path);
	public:
		//==============================
		// Public Fields
		//==============================
		// Navigation
		Ref<Rendering::Texture2D> m_Down;
		Ref<Rendering::Texture2D> m_Right;
		Ref<Rendering::Texture2D> m_Dash;
		// Modify
		Ref<Rendering::Texture2D> m_Delete;
		Ref<Rendering::Texture2D> m_Edit;
		Ref<Rendering::Texture2D> m_Cancel;
		Ref<Rendering::Texture2D> m_Cancel2;
		Ref<Rendering::Texture2D> m_Confirm;
		// Other
		Ref<Rendering::Texture2D> m_Search;
		Ref<Rendering::Texture2D> m_Checkbox_Disabled;
		Ref<Rendering::Texture2D> m_Checkbox_Enabled;
		Ref<Rendering::Texture2D> m_Notification;
		Ref<Rendering::Texture2D> m_Options;
		Ref<Rendering::Texture2D> m_Settings;
		Ref<Rendering::Texture2D> m_AI;
		Ref<Rendering::Texture2D> m_Camera;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIService;
	};

	struct SceneGraphIcons
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		SceneGraphIcons() = default;
		~SceneGraphIcons() = default;
	public:
		//==============================
		// Create Icons
		//==============================
		void LoadIcons(const std::filesystem::path& path);
	public:
		//==============================
		// Public Fields
		//==============================
		Ref<Rendering::Texture2D> m_BoxCollider;
		Ref<Rendering::Texture2D> m_CircleCollider;
		Ref<Rendering::Texture2D> m_Entity;
		Ref<Rendering::Texture2D> m_Particles;
		Ref<Rendering::Texture2D> m_ClassInstance;
		Ref<Rendering::Texture2D> m_RigidBody;
		Ref<Rendering::Texture2D> m_Tag;
		Ref<Rendering::Texture2D> m_Transform;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIService;
	};

	struct ViewportIcons
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		ViewportIcons() = default;
		~ViewportIcons() = default;
	public:
		//==============================
		// Create Icons
		//==============================
		void LoadIcons(const std::filesystem::path& path);
	public:
		//==============================
		// Public Fields
		//==============================
		Ref<Rendering::Texture2D> m_Display;
		Ref<Rendering::Texture2D> m_Simulate;
		Ref<Rendering::Texture2D> m_Step;
		Ref<Rendering::Texture2D> m_Play;
		Ref<Rendering::Texture2D> m_Pause;
		Ref<Rendering::Texture2D> m_Stop;
		Ref<Rendering::Texture2D> m_Grid;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIService;
	};

	struct RuntimeUIIcons
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		RuntimeUIIcons() = default;
		~RuntimeUIIcons() = default;
	public:
		//==============================
		// Create Icons
		//==============================
		void LoadIcons(const std::filesystem::path& path);
	public:
		//==============================
		// Public Fields
		//==============================
		Ref<Rendering::Texture2D> m_VerticalContainer;
		Ref<Rendering::Texture2D> m_HorizontalContainer;
		Ref<Rendering::Texture2D> m_DropDownWidget;
		Ref<Rendering::Texture2D> m_SliderWidget;
		Ref<Rendering::Texture2D> m_UserInterface2;
		Ref<Rendering::Texture2D> m_InputTextWidget;
		Ref<Rendering::Texture2D> m_ButtonWidget;
		Ref<Rendering::Texture2D> m_ImageButtonWidget;
		Ref<Rendering::Texture2D> m_TextWidget;
		Ref<Rendering::Texture2D> m_Window;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIService;
	};

	struct ScriptingIcons
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		ScriptingIcons() = default;
		~ScriptingIcons() = default;
	public:
		//==============================
		// Create Icons
		//==============================
		void LoadIcons(const std::filesystem::path& path);
	public:
		//==============================
		// Public Fields
		//==============================
		Ref<Rendering::Texture2D> m_Number;
		Ref<Rendering::Texture2D> m_Variable;
		Ref<Rendering::Texture2D> m_Function;
		Ref<Rendering::Texture2D> m_Boolean;
		Ref<Rendering::Texture2D> m_Decimal;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIService;
	};

	struct ContentBrowserIcons
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		ContentBrowserIcons() = default;
		~ContentBrowserIcons() = default;
	public:
		//==============================
		// Create Icons
		//==============================
		void LoadIcons(const std::filesystem::path& path);
	public:
		//==============================
		// Public Fields
		//==============================
		Ref<Rendering::Texture2D> m_Forward;
		Ref<Rendering::Texture2D> m_Directory;
		Ref<Rendering::Texture2D> m_GenericFile;
		Ref<Rendering::Texture2D> m_Back;
		Ref<Rendering::Texture2D> m_Audio;
		Ref<Rendering::Texture2D> m_Texture;
		Ref<Rendering::Texture2D> m_Binary;
		Ref<Rendering::Texture2D> m_Scene;
		Ref<Rendering::Texture2D> m_Scene_KG;
		Ref<Rendering::Texture2D> m_Registry;
		Ref<Rendering::Texture2D> m_UserInterface;
		Ref<Rendering::Texture2D> m_Font;
		Ref<Rendering::Texture2D> m_Input;
		Ref<Rendering::Texture2D> m_AI_KG;
		Ref<Rendering::Texture2D> m_Audio_KG;
		Ref<Rendering::Texture2D> m_Font_KG;
		Ref<Rendering::Texture2D> m_GlobalState;
		Ref<Rendering::Texture2D> m_ProjectComponent;
		Ref<Rendering::Texture2D> m_Script;
		Ref<Rendering::Texture2D> m_Texture_KG;
		Ref<Rendering::Texture2D> m_EmitterConfig;
		Ref<Rendering::Texture2D> m_Enum;
		Ref<Rendering::Texture2D> m_ColorPalette;
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIService;
	};

	struct ConfigFonts
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		ConfigFonts() = default;
		~ConfigFonts() = default;
	public:
		//==============================
		// Create Fonts
		//==============================
		void LoadFonts(ImGuiIO& io);
	public:
		//==============================
		// Public Fields
		//==============================
		ImFont* m_HeaderLarge{ nullptr };
		ImFont* m_HeaderRegular{ nullptr };
		ImFont* m_Title{ nullptr };
		ImFont* m_Default{ nullptr };
	private:
		//==============================
		// Owning Class
		//==============================
		friend class EditorUIService;
	};


}