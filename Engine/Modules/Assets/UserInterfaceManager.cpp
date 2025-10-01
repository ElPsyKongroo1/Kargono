#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/UserInterfaceManager.h"

#include "Modules/RuntimeUI/RuntimeUIContext.h"

namespace Kargono::Assets
{
	void UserInterfaceManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(name);

		// Create Temporary UserInterface
		Ref<RuntimeUI::UserInterface> temporaryUserInterface = CreateRef<RuntimeUI::UserInterface>();

		// Save into File
		SerializeAsset(temporaryUserInterface, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::UserInterfaceMetaData> metadata = CreateRef<Assets::UserInterfaceMetaData>();
		asset.Data.SpecificFileData = metadata;
	}

	static void SerializeSelectionData(YAML::Emitter& out, RuntimeUI::SelectionData& selectionData);
	static void SerializeImageData(YAML::Emitter& out, RuntimeUI::ImageData& imageData, const std::string& title);
	static void SerializeSingleLineTextData(YAML::Emitter& out, RuntimeUI::SingleLineTextData& textData);
	static void SerializeMultiLineTextData(YAML::Emitter& out, RuntimeUI::MultiLineTextData& textData);
	static void SerializeContainerData(YAML::Emitter& out, RuntimeUI::ContainerData& containerData);

	static void SerializeTextWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> currentWidget);
	static void SerializeButtonWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeImageWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeImageButtonWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeCheckboxWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeContainerWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeHorizontalContainerWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeVerticalContainerWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeInputTextWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeSliderWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeDropDownWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);
	static void SerializeWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget);


	void SerializeMultiLineTextData(YAML::Emitter& out, RuntimeUI::MultiLineTextData& textData)
	{
		out << YAML::Key << "Text" << YAML::Value << textData.m_Text;
		out << YAML::Key << "TextSize" << YAML::Value << textData.m_TextSize;
		out << YAML::Key << "TextColor" << YAML::Value << textData.m_TextColor;
		out << YAML::Key << "TextAlignment" << YAML::Value << Utility::ConstraintToString(textData.m_TextAlignment);
		out << YAML::Key << "TextWrapped" << YAML::Value << textData.m_TextWrapped;
	}

	void SerializeTextWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(widget.get());
		out << YAML::Key << "TextWidget" << YAML::Value;
		out << YAML::BeginMap; // Begin TextWidget Map
		SerializeMultiLineTextData(out, textWidget->m_TextData);
		out << YAML::EndMap; // End TextWidget Map
	}

	void SerializeButtonWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::ButtonWidget* buttonWidget = static_cast<RuntimeUI::ButtonWidget*>(widget.get());
		out << YAML::Key << "ButtonWidget" << YAML::Value;
		out << YAML::BeginMap; // Begin buttonWidget Map
		// Save text data
		SerializeSingleLineTextData(out, buttonWidget->m_TextData);
		// Save selection fields
		SerializeSelectionData(out, buttonWidget->m_SelectionData);
		out << YAML::EndMap; // End buttonWidget Map
	}

	void SerializeImageWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::ImageWidget* imageWidget = static_cast<RuntimeUI::ImageWidget*>(widget.get());
		out << YAML::Key << "ImageWidget" << YAML::Value;
		// Image field
		out << YAML::BeginMap; // Begin ImageWidget Map
		// Save image data
		SerializeImageData(out, imageWidget->m_ImageData, "");
		out << YAML::EndMap; // End ImageWidget Map
	}

	void SerializeImageButtonWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::ImageButtonWidget* imageButtonWidget = static_cast<RuntimeUI::ImageButtonWidget*>(widget.get());
		out << YAML::Key << "ImageButtonWidget" << YAML::Value;
		// Image field
		out << YAML::BeginMap; // Begin ImageWidget Map
		// Save image data
		SerializeImageData(out, imageButtonWidget->m_ImageData, "");
		// Save selection fields
		SerializeSelectionData(out, imageButtonWidget->m_SelectionData);
		out << YAML::EndMap; // End ImageWidget Map
	}


	void SerializeHorizontalContainerWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::HorizontalContainerWidget* containerWidget = static_cast<RuntimeUI::HorizontalContainerWidget*>(widget.get());
		out << YAML::Key << "HorizontalContainerWidget" << YAML::Value;
		// Container fields
		out << YAML::BeginMap; // Begin Container Map

		// Save unique fields
		out << YAML::Key << "ColumnWidth" << YAML::Value << containerWidget->m_ColumnWidth;
		out << YAML::Key << "ColumnSpacing" << YAML::Value << containerWidget->m_ColumnSpacing;

		// Save container data
		SerializeContainerData(out, containerWidget->m_ContainerData);
		out << YAML::EndMap; // End Container Map
	}

	void SerializeVerticalContainerWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::VerticalContainerWidget* containerWidget = static_cast<RuntimeUI::VerticalContainerWidget*>(widget.get());
		out << YAML::Key << "VerticalContainerWidget" << YAML::Value;
		// Container fields
		out << YAML::BeginMap; // Begin Container Map
		
		// Save unique fields
		out << YAML::Key << "RowHeight" << YAML::Value << containerWidget->m_RowHeight;
		out << YAML::Key << "RowSpacing" << YAML::Value << containerWidget->m_RowSpacing;

		// Save container data
		SerializeContainerData(out, containerWidget->m_ContainerData);
		out << YAML::EndMap; // End Container Map
	}

	void SerializeInputTextWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::InputTextWidget* inputTextWidget = static_cast<RuntimeUI::InputTextWidget*>(widget.get());
		out << YAML::Key << "InputTextWidget" << YAML::Value;
		out << YAML::BeginMap; // Begin InputTextWidget Map
		// Save text data
		SerializeSingleLineTextData(out, inputTextWidget->m_TextData);
		// Save selection fields
		SerializeSelectionData(out, inputTextWidget->m_SelectionData);
		// Save input text unique function pointers
		out << YAML::Key << "OnMoveCursor" << YAML::Value << (uint64_t)inputTextWidget->m_OnMoveCursorHandle;
		out << YAML::EndMap; // End InputTextWidget Map
	}

	void SerializeSliderWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::SliderWidget* sliderWidget = static_cast<RuntimeUI::SliderWidget*>(widget.get());
		out << YAML::Key << "SliderWidget" << YAML::Value;
		out << YAML::BeginMap; // Begin SliderWidget Map
		// Save selection fields
		SerializeSelectionData(out, sliderWidget->m_SelectionData);
		// Save slider unique function pointers
		out << YAML::Key << "OnMoveSlider" << YAML::Value << (uint64_t)sliderWidget->m_OnMoveSliderHandle;
		// Save other slider options
		out << YAML::Key << "Bounds" << YAML::Value << sliderWidget->m_Bounds;
		out << YAML::Key << "SliderColor" << YAML::Value << sliderWidget->m_SliderColor;
		out << YAML::Key << "LineColor" << YAML::Value << sliderWidget->m_LineColor;

		out << YAML::EndMap; // End SliderWidget Map
	}

	void SerializeDropDownWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::DropDownWidget* dropDownWidget = static_cast<RuntimeUI::DropDownWidget*>(widget.get());
		out << YAML::Key << "DropDownWidget" << YAML::Value;
		out << YAML::BeginMap; // Begin DropDownWidget Map
		// Save selection fields
		SerializeSelectionData(out, dropDownWidget->m_SelectionData);

		// Serialize list of options
		out << YAML::Key << "DropDownOptions" << YAML::Value;
		out << YAML::BeginSeq; // Begin Option Sequence
		for (RuntimeUI::SingleLineTextData& currentOption : dropDownWidget->m_DropDownOptions)
		{
			out << YAML::BeginMap; // Begin DropDown Option Map
			SerializeSingleLineTextData(out, currentOption);
			out << YAML::EndMap; // End DropDown Option Map
		}
		out << YAML::EndSeq; // End Option Sequence

		// Save drop down color
		out << YAML::Key << "DropDownBackground" << YAML::Value << dropDownWidget->m_DropDownBackground;

		// Save function pointer
		out << YAML::Key << "OnSelectOption" << YAML::Value << (uint64_t)dropDownWidget->m_OnSelectOptionHandle;

		out << YAML::EndMap; // End DropDownWidget Map
	}

	static void SerializeWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		
		switch (widget->m_WidgetType)
		{
		case RuntimeUI::WidgetTypes::TextWidget:
		{
			SerializeTextWidget(out, widget);
			break;
		}
		case RuntimeUI::WidgetTypes::ButtonWidget:
		{
			SerializeButtonWidget(out, widget);
			break;
		}
		case RuntimeUI::WidgetTypes::ImageWidget:
		{
			SerializeImageWidget(out, widget);
			break;
		}

		case RuntimeUI::WidgetTypes::ImageButtonWidget:
		{
			SerializeImageButtonWidget(out, widget);
			break;
		}
		case RuntimeUI::WidgetTypes::CheckboxWidget:
		{
			SerializeCheckboxWidget(out, widget);
			break;
		}

		case RuntimeUI::WidgetTypes::ContainerWidget:
		{
			SerializeContainerWidget(out, widget);
			break;
		}

		case RuntimeUI::WidgetTypes::HorizontalContainerWidget:
		{
			SerializeHorizontalContainerWidget(out, widget);
			break;
		}

		case RuntimeUI::WidgetTypes::VerticalContainerWidget:
		{
			SerializeVerticalContainerWidget(out, widget);
			break;
		}

		case RuntimeUI::WidgetTypes::InputTextWidget:
		{
			SerializeInputTextWidget(out, widget);
			break;
		}

		case RuntimeUI::WidgetTypes::SliderWidget:
		{
			SerializeSliderWidget(out, widget);
			break;
		}

		case RuntimeUI::WidgetTypes::DropDownWidget:
		{
			SerializeDropDownWidget(out, widget);
			break;
		}
		}
		out << YAML::EndMap; // End Widget Map
	}


	static void DeserializeMultiLineTextData(RuntimeUI::MultiLineTextData& textData, YAML::Node& node);
	static void DeserializeSingleLineTextData(RuntimeUI::SingleLineTextData& textData, const YAML::Node& node);
	static void DeserializeContainerData(RuntimeUI::ContainerData& containerData, const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static void DeserializeSelectionData(RuntimeUI::SelectionData& selectionData, YAML::Node& node);
	static void DeserializeImageData(RuntimeUI::ImageData& imageData, YAML::Node& node, const std::string& title);

	static Ref<RuntimeUI::Widget> DeserializeTextWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeButtonWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeImageWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeImageButtonWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeCheckboxWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeContainerWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeInputTextWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeSliderWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeDropDownWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);
	static Ref<RuntimeUI::Widget> DeserializeWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui);

	void DeserializeMultiLineTextData(RuntimeUI::MultiLineTextData& textData, YAML::Node& node)
	{
		textData.m_Text = node["Text"].as<std::string>();
		textData.m_TextSize = node["TextSize"].as<float>();
		textData.m_TextColor = node["TextColor"].as<glm::vec4>();
		textData.m_TextAlignment = Utility::StringToConstraint(node["TextAlignment"].as<std::string>());
		textData.m_TextWrapped = node["TextWrapped"].as<bool>();
	}



	Ref<RuntimeUI::Widget> DeserializeTextWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{	
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::TextWidget>(ui);
		YAML::Node specificWidget = node["TextWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::TextWidget;
		RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(widget.get());
		// Get multiline data
		DeserializeMultiLineTextData(textWidget->m_TextData, specificWidget);
		return widget;
	}


	Ref<RuntimeUI::Widget> DeserializeImageWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::ImageWidget>(ui);
		YAML::Node specificWidget = node["ImageWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::ImageWidget;
		RuntimeUI::ImageWidget* imageWidget = static_cast<RuntimeUI::ImageWidget*>(widget.get());
		// Get image data
		DeserializeImageData(imageWidget->m_ImageData, specificWidget, "");
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeImageButtonWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::ImageButtonWidget>(ui);
		YAML::Node specificWidget = node["ImageButtonWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::ImageButtonWidget;
		RuntimeUI::ImageButtonWidget* imageButtonWidget = static_cast<RuntimeUI::ImageButtonWidget*>(widget.get());
		// Get selection data
		DeserializeSelectionData(imageButtonWidget->m_SelectionData, specificWidget);
		// Get image data
		DeserializeImageData(imageButtonWidget->m_ImageData, specificWidget, "");
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeCheckboxWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::CheckboxWidget>(ui);
		widget->m_WidgetType = RuntimeUI::WidgetTypes::CheckboxWidget;
		YAML::Node specificWidget = node["CheckboxWidget"];
		RuntimeUI::CheckboxWidget* checkboxWidget = static_cast<RuntimeUI::CheckboxWidget*>(widget.get());
		// Get checked status
		checkboxWidget->m_Checked = specificWidget["Checked"].as<bool>();
		// Get selection data
		DeserializeSelectionData(checkboxWidget->m_SelectionData, specificWidget);
		// Get checked image data
		DeserializeImageData(checkboxWidget->m_ImageChecked, specificWidget, "Checked");
		// Get unchecked image data
		DeserializeImageData(checkboxWidget->m_ImageUnChecked, specificWidget, "UnChecked");
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeHorizontalContainerWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::HorizontalContainerWidget>(ui);
		YAML::Node specificWidget = node["HorizontalContainerWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::HorizontalContainerWidget;
		RuntimeUI::HorizontalContainerWidget* HorizontalContainerWidget = static_cast<RuntimeUI::HorizontalContainerWidget*>(widget.get());
		// Get unique data
		HorizontalContainerWidget->m_ColumnWidth = specificWidget["ColumnWidth"].as<float>();
		HorizontalContainerWidget->m_ColumnSpacing = specificWidget["ColumnSpacing"].as<float>();
		// Get container data
		DeserializeContainerData(HorizontalContainerWidget->m_ContainerData, specificWidget, ui);
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeVerticalContainerWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::VerticalContainerWidget>(ui);
		YAML::Node specificWidget = node["VerticalContainerWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::VerticalContainerWidget;
		RuntimeUI::VerticalContainerWidget* verticalContainerWidget = static_cast<RuntimeUI::VerticalContainerWidget*>(widget.get());
		// Get unique data
		verticalContainerWidget->m_RowHeight = specificWidget["RowHeight"].as<float>();
		verticalContainerWidget->m_RowSpacing = specificWidget["RowSpacing"].as<float>();
		// Get container data
		DeserializeContainerData(verticalContainerWidget->m_ContainerData, specificWidget, ui);
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeInputTextWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::InputTextWidget>(ui);
		YAML::Node specificWidget = node["InputTextWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::InputTextWidget;
		RuntimeUI::InputTextWidget* inputTextWidget = static_cast<RuntimeUI::InputTextWidget*>(widget.get());
		// Get single line data
		DeserializeSingleLineTextData(inputTextWidget->m_TextData, specificWidget);
		// Get selection data
		DeserializeSelectionData(inputTextWidget->m_SelectionData, specificWidget);
		// Get input map specific function pointers
		inputTextWidget->m_OnMoveCursorHandle = specificWidget["OnMoveCursor"].as<uint64_t>();
		if (inputTextWidget->m_OnMoveCursorHandle == Assets::k_EmptyHandle)
		{
			inputTextWidget->m_OnMoveCursor = nullptr;
		}
		else
		{
			Ref<Scripting::Script> onPressScript = Assets::AssetService::GetScript(inputTextWidget->m_OnMoveCursorHandle);
			if (!onPressScript)
			{
				KG_WARN("Unable to locate On Move Cursor Script!");
				return nullptr;
			}
			inputTextWidget->m_OnMoveCursor = onPressScript;
		}
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeSliderWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::SliderWidget>(ui);
		YAML::Node specificWidget = node["SliderWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::SliderWidget;
		RuntimeUI::SliderWidget* sliderWidget = static_cast<RuntimeUI::SliderWidget*>(widget.get());
		// Get selection data
		DeserializeSelectionData(sliderWidget->m_SelectionData, specificWidget);

		// Get slider specific fields
		sliderWidget->m_Bounds = specificWidget["Bounds"].as<Math::vec2>();
		sliderWidget->m_SliderColor = specificWidget["SliderColor"].as<Math::vec4>();
		sliderWidget->m_LineColor = specificWidget["LineColor"].as<Math::vec4>();

		// Get slider widget specific function pointers
		sliderWidget->m_OnMoveSliderHandle = specificWidget["OnMoveSlider"].as<uint64_t>();
		if (sliderWidget->m_OnMoveSliderHandle == Assets::k_EmptyHandle)
		{
			sliderWidget->m_OnMoveSlider = nullptr;
		}
		else
		{
			Ref<Scripting::Script> onPressScript = Assets::AssetService::GetScript(sliderWidget->m_OnMoveSliderHandle);
			if (!onPressScript)
			{
				KG_WARN("Unable to locate on move slider Script!");
				return nullptr;
			}
			sliderWidget->m_OnMoveSlider = onPressScript;
		}
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeDropDownWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::DropDownWidget>(ui);
		YAML::Node specificWidget = node["DropDownWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::DropDownWidget;
		RuntimeUI::DropDownWidget* dropDownWidget = static_cast<RuntimeUI::DropDownWidget*>(widget.get());
		// Get selection data
		DeserializeSelectionData(dropDownWidget->m_SelectionData, specificWidget);

		// Get drop-down specific fields
		dropDownWidget->m_DropDownBackground = specificWidget["DropDownBackground"].as<Math::vec4>();

		// Get all drop down options
		YAML::Node dropDownOptionsNode = specificWidget["DropDownOptions"];
		for (const YAML::Node& optionNode : dropDownOptionsNode)
		{
			RuntimeUI::SingleLineTextData& newTextData = dropDownWidget->m_DropDownOptions.emplace_back(RuntimeUI::SingleLineTextData());
			DeserializeSingleLineTextData(newTextData, optionNode);
		}

		// Get slider widget specific function pointers
		dropDownWidget->m_OnSelectOptionHandle = specificWidget["OnSelectOption"].as<uint64_t>();
		if (dropDownWidget->m_OnSelectOptionHandle == Assets::k_EmptyHandle)
		{
			dropDownWidget->m_OnSelectOption = nullptr;
		}
		else
		{
			Ref<Scripting::Script> onPressScript = Assets::AssetService::GetScript(dropDownWidget->m_OnSelectOptionHandle);
			if (!onPressScript)
			{
				KG_WARN("Unable to locate on select option Script!");
				return nullptr;
			}
			dropDownWidget->m_OnSelectOption = onPressScript;
		}
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeWidget(const YAML::Node& node, RuntimeUI::UserInterface* ui)
	{
		Ref<RuntimeUI::Widget> widget;
		RuntimeUI::WidgetTypes widgetType = Utility::StringToWidgetType(node["WidgetType"].as<std::string>());
		switch (widgetType)
		{
		case RuntimeUI::WidgetTypes::TextWidget:
		{
			widget = DeserializeTextWidget(node, ui);
			break;
		}
		case RuntimeUI::WidgetTypes::ButtonWidget:
		{
			widget = DeserializeButtonWidget(node, ui);
			break;
		}
		case RuntimeUI::WidgetTypes::ImageWidget:
		{
			widget = DeserializeImageWidget(node, ui);
			break;
		}
		case RuntimeUI::WidgetTypes::ImageButtonWidget:
		{
			widget = DeserializeImageButtonWidget(node, ui);
			break;
		}
		case RuntimeUI::WidgetTypes::CheckboxWidget:
		{
			widget = DeserializeCheckboxWidget(node, ui);
			break;
		}
		case RuntimeUI::WidgetTypes::ContainerWidget:
		{
			widget = DeserializeContainerWidget(node, ui);
			break;
		}
		case RuntimeUI::WidgetTypes::HorizontalContainerWidget:
		{
			widget = DeserializeHorizontalContainerWidget(node, ui);
			break;
		}
		case RuntimeUI::WidgetTypes::VerticalContainerWidget:
		{
			widget = DeserializeVerticalContainerWidget(node, ui);
			break;
		}
		case RuntimeUI::WidgetTypes::InputTextWidget:
		{
			widget = DeserializeInputTextWidget(node, ui);
			break;
		}

		case RuntimeUI::WidgetTypes::SliderWidget:
		{
			widget = DeserializeSliderWidget(node, ui);
			break;
		}

		case RuntimeUI::WidgetTypes::DropDownWidget:
		{
			widget = DeserializeDropDownWidget(node, ui);
			break;
		}
		default:
		{
			KG_WARN("Invalid Widget Type in UserInterface Deserialization");
			return nullptr;
		}
		}



		return widget;
	}

	static bool RemoveScriptFromWidget(Ref<RuntimeUI::Widget> widgetRef, Assets::AssetHandle scriptHandle);

	bool RemoveScriptFromWidget(Ref<RuntimeUI::Widget> widgetRef, Assets::AssetHandle scriptHandle)
	{
		bool uiModified{ false };

		if (widgetRef->m_WidgetType == RuntimeUI::WidgetTypes::InputTextWidget)
		{
			RuntimeUI::InputTextWidget& inputTextWidget = *(RuntimeUI::InputTextWidget*)widgetRef.get();
			if (inputTextWidget.m_OnMoveCursorHandle == scriptHandle)
			{
				inputTextWidget.m_OnMoveCursorHandle = Assets::k_EmptyHandle;
				inputTextWidget.m_OnMoveCursor = nullptr;
				uiModified = true;
			}
		}

		if (widgetRef->m_WidgetType == RuntimeUI::WidgetTypes::SliderWidget)
		{
			RuntimeUI::SliderWidget& sliderWidget = *(RuntimeUI::SliderWidget*)widgetRef.get();
			if (sliderWidget.m_OnMoveSliderHandle == scriptHandle)
			{
				sliderWidget.m_OnMoveSliderHandle = Assets::k_EmptyHandle;
				sliderWidget.m_OnMoveSlider = nullptr;
				uiModified = true;
			}
		}

		if (widgetRef->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget)
		{
			RuntimeUI::DropDownWidget& dropDownWidget = *(RuntimeUI::DropDownWidget*)widgetRef.get();
			if (dropDownWidget.m_OnSelectOptionHandle == scriptHandle)
			{
				dropDownWidget.m_OnSelectOptionHandle = Assets::k_EmptyHandle;
				dropDownWidget.m_OnSelectOption = nullptr;
				uiModified = true;
			}
		}

		RuntimeUI::ContainerData* containerData = widgetRef->GetContainerData();
		if (containerData)
		{
			for (Ref<RuntimeUI::Widget> currentWidget : containerData->m_ContainedWidgets)
			{
				bool modified = RemoveScriptFromWidget(currentWidget, scriptHandle);
				if (modified)
				{
					uiModified = true;
				}
			}
		}

		// Check if this widget has a selection data
		RuntimeUI::SelectionData* selectionData = widgetRef->GetSelectionData();
		if (!selectionData)
		{
			return uiModified;
		}

		// Remove script references from widget if necessary
		if (selectionData->m_FunctionPointers.m_OnPressHandle == scriptHandle)
		{
			selectionData->m_FunctionPointers.m_OnPressHandle = Assets::k_EmptyHandle;
			selectionData->m_FunctionPointers.m_OnPress = nullptr;
			uiModified = true;
		}

		return uiModified;
	}

	bool UserInterfaceManager::RemoveScript(Ref<RuntimeUI::UserInterface> userInterfaceRef, Assets::AssetHandle scriptHandle)
	{
		// Handle UI level function pointers
		bool uiModified{ false };
		if (userInterfaceRef->m_Config.m_FunctionPointers.m_OnMoveHandle == scriptHandle)
		{
			userInterfaceRef->m_Config.m_FunctionPointers.m_OnMoveHandle = Assets::k_EmptyHandle;
			userInterfaceRef->m_Config.m_FunctionPointers.m_OnMove = nullptr;
			uiModified = true;
		}
		if (userInterfaceRef->m_Config.m_FunctionPointers.m_OnHoverHandle == scriptHandle)
		{
			userInterfaceRef->m_Config.m_FunctionPointers.m_OnHoverHandle = Assets::k_EmptyHandle;
			userInterfaceRef->m_Config.m_FunctionPointers.m_OnHover = nullptr;
			uiModified = true;
		}

		// Handle all widgets in all windows
		for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_WindowsState.m_Windows)
		{
			for (Ref<RuntimeUI::Widget> widgetRef : currentWindow.m_Widgets)
			{
				bool modified = RemoveScriptFromWidget(widgetRef, scriptHandle);

				if (modified)
				{
					uiModified = true;
				}
			}
		}

		return uiModified;
	}

	static bool RemoveTextureFromWidget(Ref<RuntimeUI::Widget> widgetRef, Assets::AssetHandle textureHandle);

	bool RemoveTextureFromWidget(Ref<RuntimeUI::Widget> widgetRef, Assets::AssetHandle textureHandle)
	{
		bool uiModified{ false };

		if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::ImageWidget)
		{
			// Remove texture reference from widget if necessary
			RuntimeUI::ImageWidget& imageWidget = *(RuntimeUI::ImageWidget*)widgetRef.get();
			if (imageWidget.m_ImageData.m_ImageHandle == textureHandle)
			{
				RuntimeUI::ImageWidget& buttonWidget = *(RuntimeUI::ImageWidget*)widgetRef.get();
				buttonWidget.m_ImageData.m_ImageHandle = Assets::k_EmptyHandle;
				buttonWidget.m_ImageData.m_ImageRef = nullptr;
				uiModified = true;
			}
		}
		if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::ImageButtonWidget)
		{
			// Remove texture reference from widget if necessary
			RuntimeUI::ImageButtonWidget& imageWidget = *(RuntimeUI::ImageButtonWidget*)widgetRef.get();
			if (imageWidget.m_ImageData.m_ImageHandle == textureHandle)
			{
				RuntimeUI::ImageButtonWidget& imageButtonWidget = *(RuntimeUI::ImageButtonWidget*)widgetRef.get();
				imageButtonWidget.m_ImageData.m_ImageHandle = Assets::k_EmptyHandle;
				imageButtonWidget.m_ImageData.m_ImageRef = nullptr;
				uiModified = true;
			}
		}
		if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::CheckboxWidget)
		{
			// Remove texture reference from widget if necessary
			RuntimeUI::CheckboxWidget& checkboxWidget = *(RuntimeUI::CheckboxWidget*)widgetRef.get();
			if (checkboxWidget.m_ImageUnChecked.m_ImageHandle == textureHandle)
			{
				checkboxWidget.m_ImageUnChecked.m_ImageHandle = Assets::k_EmptyHandle;
				checkboxWidget.m_ImageUnChecked.m_ImageRef = nullptr;
				uiModified = true;
			}
			if (checkboxWidget.m_ImageChecked.m_ImageHandle == textureHandle)
			{
				checkboxWidget.m_ImageChecked.m_ImageHandle = Assets::k_EmptyHandle;
				checkboxWidget.m_ImageChecked.m_ImageRef = nullptr;
				uiModified = true;
			}
		}

		RuntimeUI::ContainerData* containerData = widgetRef->GetContainerData();
		if (containerData)
		{
			for (Ref<RuntimeUI::Widget> currentWidget : containerData->m_ContainedWidgets)
			{
				bool modified = RemoveTextureFromWidget(currentWidget, textureHandle);
				if (modified)
				{
					uiModified = true;
				}
			}
		}

		return uiModified;
	}

	bool UserInterfaceManager::RemoveTexture(Ref<RuntimeUI::UserInterface> userInterfaceRef, Assets::AssetHandle textureHandle)
	{
		bool uiModified{ false };

		// Handle all widgets in all windows
		for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_WindowsState.m_Windows)
		{
			for (Ref<RuntimeUI::Widget> widgetRef : currentWindow.m_Widgets)
			{
				bool modified = RemoveTextureFromWidget(widgetRef, textureHandle);
				if (modified)
				{
					uiModified = true;
				}
				
			}
		}

		return uiModified;
	}
}
