#pragma once
#include "Modules/EditorUI/Widgets/EditorUIWidget.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Core/UUID.h"

#include <functional>

namespace Kargono::EditorUI
{
	struct OptionEntry
	{
	public:
		//==============================
		// Operator Overload(s)
		//==============================
		bool operator==(const OptionEntry& other) const
		{
			if (this->m_Label == other.m_Label && this->m_Handle == other.m_Handle)
			{
				return true;
			}
			return false;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		UUID m_Handle{ k_EmptyUUID };
	};

	using OptionIndex = size_t;
	constexpr OptionIndex k_InvalidEntryIndex{ std::numeric_limits<OptionIndex>().max() };

	class OptionList
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		OptionList() = default;
		~OptionList() = default;
	public:
		//==============================
		// Modify Options
		//==============================
		OptionEntry* CreateOption();
		void Clear();
	public:
		//==============================
		// Query Options
		//==============================
		OptionEntry* GetOption(OptionIndex index);
	public:
		//==============================
		// Forward Iterator Begin/End (allow for-loops and other algs)
		//==============================
		std::vector<OptionEntry>::iterator begin()
		{
			return m_Options.begin();
		}
		std::vector<OptionEntry>::iterator end()
		{
			return m_Options.end();
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<OptionEntry> m_Options{};
	};

	enum SelectOptionFlags : WidgetFlags
	{
		SelectOption_None = 0,
		SelectOption_Indented = BIT(0), // Indents the text (used in collapsing headers usually)
		SelectOption_PopupOnly = BIT(1), // Determines if line of text is generated for options
		SelectOption_HandleEditButtonExternally = BIT(2) // Allows calling a custom function for edit button
	};

	using OptionMap = std::unordered_map<std::string, std::vector<OptionEntry>>; // TODO: Bruh what is this heap garbage

	struct SelectOptionWidget : public Widget
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		SelectOptionWidget() : Widget() {}
		~SelectOptionWidget() = default;

	public:
		//==============================
		// Rendering
		//==============================
		void RenderOptions();

	private:
		// Helper functions
		OptionMap GenerateSearchCache(OptionMap& originalList, const char* searchQuery);
	public:
		//==============================
		// Modify Options
		//==============================
		// Add
		void AddToOptions(const std::string& group, const std::string& optionLabel, UUID optionIdentifier);
		// Clear/delete
		void ClearOptions();

	public:
		//==============================
		// Query Options
		//==============================
		OptionMap& GetAllOptions()
		{
			return m_ActiveOptions;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		FixedString32 m_Label{};
		OptionEntry m_CurrentOption{};
		uint32_t m_LineCount{ 3 };
		std::function<void(SelectOptionWidget&)> m_PopupAction{ nullptr };
		std::function<void(const OptionEntry&)> m_ConfirmAction{ nullptr };
		WidgetFlags m_Flags{ SelectOption_None };
		Ref<void> m_ProvidedData{ nullptr };
		// Only used if PopupOnly is true
		bool m_OpenPopup{ false };
		// Only used if HandleEditButtonExternally is true
		std::function<void(SelectOptionWidget&)> m_OnEdit{ nullptr };

	private:
		//==============================
		// Internal Fields
		//==============================
		OptionMap m_ActiveOptions{};
		bool m_Searching{ false };
		OptionEntry m_CachedSelection{};
		OptionMap m_CachedSearchResults{};
	};
}


