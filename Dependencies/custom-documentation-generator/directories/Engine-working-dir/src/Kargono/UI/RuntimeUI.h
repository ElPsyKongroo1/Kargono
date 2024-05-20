#pragma once

#include "Kargono/Renderer/EditorCamera.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Text/TextEngine.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Math/Math.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>


namespace Kargono
{
	class UIEngine
	{
	public:

		struct EventCallbacks
		{
			std::function<void()> OnMouseClick;
		};

		enum class WidgetTypes
		{
			None = 0, TextWidget, ButtonWidget, CheckboxWidget, ComboWidget, PopupWidget
		};

		static std::string WidgetTypeToString(WidgetTypes widgetType)
		{
			switch(widgetType)
			{
				case WidgetTypes::TextWidget: return "TextWidget";
				case WidgetTypes::ButtonWidget: return "ButtonWidget";
				case WidgetTypes::CheckboxWidget: return "CheckboxWidget";
				case WidgetTypes::ComboWidget: return "ComboWidget";
				case WidgetTypes::PopupWidget: return "PopupWidget";
				case WidgetTypes::None: return "None";
				default:
				{
				KG_CORE_ASSERT(false, "Invalid Widget Type at WidgetTypeToString");
				return "None";
				}
			}
		}

		static WidgetTypes StringToWidgetType(const std::string& widgetName)
		{
			if (widgetName == "TextWidget") { return WidgetTypes::TextWidget; }
			if (widgetName == "ButtonWidget") { return WidgetTypes::ButtonWidget; }
			if (widgetName == "CheckboxWidget") { return WidgetTypes::CheckboxWidget; }
			if (widgetName == "ComboWidget") { return WidgetTypes::ComboWidget; }
			if (widgetName == "PopupWidget") { return WidgetTypes::PopupWidget; }
			if (widgetName == "None") { return WidgetTypes::None; }

			KG_CORE_ASSERT(false, "Invalid Widget Type at StringToWidgetType");
			return WidgetTypes::None;
		}

		class Widget
		{
		public:
			virtual ~Widget() = default;
		public:
			virtual void PushRenderData(Math::vec3 translation, const Math::vec3& scale, float viewportWidth) = 0;
		public:
			Math::vec2 WindowPosition {0.4f};
			Math::vec2 Size  {0.3f};
			Math::vec4 BackgroundColor{1.0f};
			EventCallbacks FunctionPointers;
			WidgetTypes WidgetType {WidgetTypes::None};
		};


		class TextWidget : public Widget
		{
		public:
			TextWidget()
			: Widget()
			{
				WidgetType = WidgetTypes::TextWidget;
				Size = { 0.3f, 0.1f };
				CalculateTextSize();
			}
			virtual ~TextWidget() override = default;
		public:
			virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
		private:
			void CalculateTextSize();

		public:
			void SetText(const std::string& newText);

			std::string Text{ "New Text" };
			float TextSize{ 0.12f };
			Math::vec2 TextAbsoluteDimensions {};
			bool TextCentered = true;
		};

		class ButtonWidget : public Widget
		{
		public:
			ButtonWidget()
				: Widget()
			{
				WidgetType = WidgetTypes::ButtonWidget;
			}
			virtual ~ButtonWidget() override = default;
		public:
			virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
		};

		class CheckboxWidget : public Widget
		{
		public:
			CheckboxWidget()
				: Widget()
			{
				WidgetType = WidgetTypes::CheckboxWidget;
			}
			virtual ~CheckboxWidget() override = default;
		public:
			virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
		};

		class ComboWidget : public Widget
		{
		public:
			ComboWidget()
				: Widget()
			{
				WidgetType = WidgetTypes::ComboWidget;
			}
			virtual ~ComboWidget() override = default;
		public:
			virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
		};

		class PopupWidget : public Widget
		{
		public:
			PopupWidget()
				: Widget()
			{
				WidgetType = WidgetTypes::PopupWidget;
			}
			virtual ~PopupWidget() override = default;
		public:
			virtual void PushRenderData(Math::vec3 windowTranslation, const Math::vec3& windowSize, float viewportWidth) override;
		};

		struct Window
		{
			struct WidgetCounts
			{
				uint16_t TextWidgetCount{ 0 };
				uint16_t TextWidgetLocation {};
				uint16_t ButtonWidgetCount{ 0 };
				uint16_t ButtonWidgetLocation {};
				uint16_t CheckboxWidgetCount{ 0 };
				uint16_t CheckboxWidgetLocation {};
				uint16_t ComboWidgetCount{ 0 };
				uint16_t ComboWidgetLocation {};
				uint16_t PopupWidgetCount{ 0 };
				uint16_t PopupWidgetLocation {};
			};
		public:
			Math::vec3 ScreenPosition {};
			Math::vec2 Size {};
			Math::vec4 BackgroundColor {1.0f};
			WidgetCounts WidgetCounts{};
			int32_t ParentIndex{ -1 };
			int32_t ChildBufferIndex{ -1 };
			uint32_t ChildBufferSize{ 0 };
			std::vector<Ref<UIEngine::Widget>> Widgets {};


			void DisplayWindow();
			void HideWindow();
			bool GetWindowDisplayed();

		private:
			bool WindowDisplayed = false;

		public:
			void AddTextWidget(Ref<UIEngine::TextWidget> newWidget);
			void AddButtonWidget(Ref<UIEngine::ButtonWidget> newWidget);
			void AddCheckboxWidget(Ref<UIEngine::CheckboxWidget> newWidget);
			void AddComboWidget(Ref<UIEngine::ComboWidget> newWidget);
			void AddPopupWidget(Ref<UIEngine::PopupWidget> newWidget);
			void DeleteWidget(int32_t widgetLocation);
		private:
			void IncrementIterators(uint16_t iterator);
			void DecrementIterators(uint16_t iterator);
		};

		struct UIObject
		{
			std::vector<UIEngine::Window> Windows {};
			Ref<Font> m_Font = nullptr;
			Assets::AssetHandle m_FontHandle {0};
		};
	public:
		static void Init();
		static void LoadUIObject(Ref<UIObject> uiObject, Assets::AssetHandle uiHandle);
		static void Terminate();
		static bool SaveCurrentUI();
		static void DeleteWindow(uint32_t windowLocation);
		static void PushRenderData(const EditorCamera& camera, uint32_t viewportWidth = 0, uint32_t viewportHeight = 0);
		static void AddWindow(UIEngine::Window& window); // TODO: Remove this api, it is for testing!
		static void SetFont(Ref<Font> newFont, Assets::AssetHandle fontHandle);
		static std::vector<UIEngine::Window>& GetAllWindows();
		static void RefreshDisplayedWindows();
		static void ClearUIEngine();
		static Ref<UIEngine::UIObject> GetCurrentUIObject();
		static Assets::AssetHandle GetCurrentUIHandle();
		static void SetCurrentUIObject(Ref<UIEngine::UIObject> newUI);
		static void SetCurrentUIHandle(Assets::AssetHandle newHandle);
	private:
		std::vector<UIEngine::Window*> m_DisplayedWindows{};
		Ref<UIEngine::UIObject> m_CurrentUI{ nullptr };
		Assets::AssetHandle m_CurrentUIHandle{0};
		std::vector<UIEngine::Window> m_UICache {};
		UIEngine::Widget* m_SelectedWidget { nullptr };
		UIEngine::Widget* m_HoveredWidget { nullptr };
		UIEngine::Window* m_ActiveWindow { nullptr };

		Ref<Font> m_CurrentFont;
		Assets::AssetHandle m_FontHandle {0};

		static UIEngine s_Engine;
	};
}


