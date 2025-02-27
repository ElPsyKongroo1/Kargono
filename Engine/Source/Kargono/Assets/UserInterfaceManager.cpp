#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/UserInterfaceManager.h"

#include "Kargono/RuntimeUI/RuntimeUI.h"

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

	void SerializeSelectionData(YAML::Emitter& out, RuntimeUI::SelectionData& selectionData)
	{
		// Color fields
		out << YAML::Key << "DefaultBackgroundColor" << YAML::Value << selectionData.m_DefaultBackgroundColor;
		// Selectable fields
		out << YAML::Key << "Selectable" << YAML::Value << selectionData.m_Selectable;
		// Function pointer fields
		out << YAML::Key << "FunctionPointerOnPress" << YAML::Value << (uint64_t)selectionData.m_FunctionPointers.m_OnPressHandle;
	}
	void SerializeImageData(YAML::Emitter& out, RuntimeUI::ImageData& imageData, const std::string& title)
	{
		out << YAML::Key << (title + "Image") << YAML::Value << (uint64_t)imageData.m_ImageHandle;
		out << YAML::Key << (title + "FixedAspectRatio") << YAML::Value << imageData.m_FixedAspectRatio;
	}
	void SerializeSingleLineTextData(YAML::Emitter& out, RuntimeUI::SingleLineTextData& textData)
	{
		out << YAML::Key << "Text" << YAML::Value << textData.m_Text;
		out << YAML::Key << "TextSize" << YAML::Value << textData.m_TextSize;
		out << YAML::Key << "TextColor" << YAML::Value << textData.m_TextColor;
		out << YAML::Key << "TextAlignment" << YAML::Value << Utility::ConstraintToString(textData.m_TextAlignment);
	}
	void SerializeMultiLineTextData(YAML::Emitter& out, RuntimeUI::MultiLineTextData& textData)
	{
		out << YAML::Key << "Text" << YAML::Value << textData.m_Text;
		out << YAML::Key << "TextSize" << YAML::Value << textData.m_TextSize;
		out << YAML::Key << "TextColor" << YAML::Value << textData.m_TextColor;
		out << YAML::Key << "TextAlignment" << YAML::Value << Utility::ConstraintToString(textData.m_TextAlignment);
		out << YAML::Key << "TextWrapped" << YAML::Value << textData.m_TextWrapped;
	}

	void SerializeContainerData(YAML::Emitter& out, RuntimeUI::ContainerData& containerData)
	{
		out << YAML::Key << "BackgroundColor" << YAML::Value << containerData.m_BackgroundColor;
		out << YAML::Key << "ContainerWidgets" << YAML::Value << YAML::BeginSeq; // Start container widgets sequence
		for (Ref<RuntimeUI::Widget> widget : containerData.m_ContainedWidgets)
		{
			SerializeWidget(out, widget);
		}
		out << YAML::EndSeq; // End container widgets sequence
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


	void SerializeCheckboxWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::CheckboxWidget* checkboxWidget = static_cast<RuntimeUI::CheckboxWidget*>(widget.get());
		out << YAML::Key << "CheckboxWidget" << YAML::Value;
		// Image fields
		out << YAML::BeginMap; // Begin Checkbox Map
		// Save checked
		out << YAML::Key << "Checked" << YAML::Value << checkboxWidget->m_Checked;
		// Save image data
		SerializeImageData(out, checkboxWidget->m_ImageChecked, "Checked");
		// Save image data
		SerializeImageData(out, checkboxWidget->m_ImageUnChecked, "UnChecked");
		// Save selection fields
		SerializeSelectionData(out, checkboxWidget->m_SelectionData);
		out << YAML::EndMap; // End Checkbox Map
	}


	void SerializeContainerWidget(YAML::Emitter& out, Ref<RuntimeUI::Widget> widget)
	{
		RuntimeUI::ContainerWidget* containerWidget = static_cast<RuntimeUI::ContainerWidget*>(widget.get());
		out << YAML::Key << "ContainerWidget" << YAML::Value;
		// Container fields
		out << YAML::BeginMap; // Begin Container Map
		// Save container data
		SerializeContainerData(out, containerWidget->m_ContainerData);
		out << YAML::EndMap; // End Container Map
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
		out << YAML::BeginMap; // Begin Widget Map

		out << YAML::Key << "Tag" << YAML::Value << widget->m_Tag;
		out << YAML::Key << "ID" << YAML::Value << widget->m_ID;
		out << YAML::Key << "XRelativeOrAbsolute" << YAML::Value << Utility::RelativeOrAbsoluteToString(widget->m_XRelativeOrAbsolute);
		out << YAML::Key << "YRelativeOrAbsolute" << YAML::Value << Utility::RelativeOrAbsoluteToString(widget->m_YRelativeOrAbsolute);
		out << YAML::Key << "XConstraint" << YAML::Value << Utility::ConstraintToString(widget->m_XConstraint);
		out << YAML::Key << "YConstraint" << YAML::Value << Utility::ConstraintToString(widget->m_YConstraint);
		out << YAML::Key << "PercentPosition" << YAML::Value << widget->m_PercentPosition;
		out << YAML::Key << "PixelPosition" << YAML::Value << widget->m_PixelPosition;
		out << YAML::Key << "SizeType" << YAML::Value << Utility::PixelOrPercentToString(widget->m_SizeType);
		out << YAML::Key << "XPositionType" << YAML::Value << Utility::PixelOrPercentToString(widget->m_XPositionType);
		out << YAML::Key << "YPositionType" << YAML::Value << Utility::PixelOrPercentToString(widget->m_YPositionType);
		out << YAML::Key << "PercentSize" << YAML::Value << widget->m_PercentSize;
		out << YAML::Key << "PixelSize" << YAML::Value << widget->m_PixelSize;
		out << YAML::Key << "WidgetType" << YAML::Value << Utility::WidgetTypeToString(widget->m_WidgetType);
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


	void UserInterfaceManager::SerializeAsset(Ref<RuntimeUI::UserInterface> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Select Color
		out << YAML::Key << "SelectColor" << YAML::Value << assetReference->m_SelectColor;
		out << YAML::Key << "HoveredColor" << YAML::Value << assetReference->m_HoveredColor;
		out << YAML::Key << "EditingColor" << YAML::Value << assetReference->m_EditingColor;

		// Function Pointers
		out << YAML::Key << "FunctionPointerOnMove" << YAML::Value << (uint64_t)assetReference->m_FunctionPointers.m_OnMoveHandle;
		out << YAML::Key << "FunctionPointerOnHover" << YAML::Value << (uint64_t)assetReference->m_FunctionPointers.m_OnHoverHandle;
		// Font
		out << YAML::Key << "Font" << YAML::Value << static_cast<uint64_t>(assetReference->m_FontHandle);
		// Windows
		out << YAML::Key << "Windows" << YAML::Value;
		out << YAML::BeginSeq; // Start of Windows Seq

		for (RuntimeUI::Window& window : assetReference->m_Windows)
		{
			out << YAML::BeginMap; // Start Window Map

			out << YAML::Key << "Tag" << YAML::Value << window.m_Tag;
			out << YAML::Key << "ID" << YAML::Value << window.m_ID;
			out << YAML::Key << "ScreenPosition" << YAML::Value << window.m_ScreenPosition;
			out << YAML::Key << "Size" << YAML::Value << window.m_Size;
			out << YAML::Key << "BackgroundColor" << YAML::Value << window.m_BackgroundColor;
			out << YAML::Key << "DefaultActiveWidget" << YAML::Value << window.m_DefaultActiveWidget;

			out << YAML::Key << "Widgets" << YAML::Value;
			out << YAML::BeginSeq; // Begin Widget Sequence

			for (Ref<RuntimeUI::Widget> widget : window.m_Widgets)
			{
				SerializeWidget(out, widget);
			}

			out << YAML::EndSeq; // End Widget Sequence

			out << YAML::EndMap; // End Window Map
			std::vector<Ref<RuntimeUI::Widget>> Widgets {};
		}

		out << YAML::EndSeq; // End of Windows Seq
		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
	}

	static void DeserializeMultiLineTextData(RuntimeUI::MultiLineTextData& textData, YAML::Node& node);
	static void DeserializeSingleLineTextData(RuntimeUI::SingleLineTextData& textData, const YAML::Node& node);
	static void DeserializeContainerData(RuntimeUI::ContainerData& containerData, const YAML::Node& node);
	static void DeserializeSelectionData(RuntimeUI::SelectionData& selectionData, YAML::Node& node);
	static void DeserializeImageData(RuntimeUI::ImageData& imageData, YAML::Node& node, const std::string& title);

	static Ref<RuntimeUI::Widget> DeserializeTextWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeButtonWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeImageWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeImageButtonWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeCheckboxWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeContainerWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeInputTextWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeSliderWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeDropDownWidget(const YAML::Node& node);
	static Ref<RuntimeUI::Widget> DeserializeWidget(const YAML::Node& node);

	void DeserializeMultiLineTextData(RuntimeUI::MultiLineTextData& textData, YAML::Node& node)
	{
		textData.m_Text = node["Text"].as<std::string>();
		textData.m_TextSize = node["TextSize"].as<float>();
		textData.m_TextColor = node["TextColor"].as<glm::vec4>();
		textData.m_TextAlignment = Utility::StringToConstraint(node["TextAlignment"].as<std::string>());
		textData.m_TextWrapped = node["TextWrapped"].as<bool>();
	}
	void DeserializeSingleLineTextData(RuntimeUI::SingleLineTextData& textData, const YAML::Node& node)
	{
		// Text fields
		textData.m_Text = node["Text"].as<std::string>();
		textData.m_TextSize = node["TextSize"].as<float>();
		textData.m_TextColor = node["TextColor"].as<glm::vec4>();
		textData.m_TextAlignment = Utility::StringToConstraint(node["TextAlignment"].as<std::string>());
	}

	void DeserializeContainerData(RuntimeUI::ContainerData& containerData, const YAML::Node& node)
	{
		// Deserialize background color
		containerData.m_BackgroundColor = node["BackgroundColor"].as<Math::vec4>();

		// Deserialize all child widgets
		YAML::Node containerWidgetNodes = node["ContainerWidgets"];
		for (YAML::Node containerWidgetNode : containerWidgetNodes)
		{
			Ref<RuntimeUI::Widget> newWidget = DeserializeWidget(containerWidgetNode);
			containerData.m_ContainedWidgets.push_back(newWidget);
		}
	}
	void DeserializeSelectionData(RuntimeUI::SelectionData& selectionData, YAML::Node& node)
	{
		// Color fields
		selectionData.m_DefaultBackgroundColor = node["DefaultBackgroundColor"].as<Math::vec4>();
		// Selectable field
		selectionData.m_Selectable = node["Selectable"].as<bool>();
		// Function pointer fields
		selectionData.m_FunctionPointers.m_OnPressHandle = node["FunctionPointerOnPress"].as<uint64_t>();
		if (selectionData.m_FunctionPointers.m_OnPressHandle == Assets::EmptyHandle)
		{
			selectionData.m_FunctionPointers.m_OnPress = nullptr;
		}
		else
		{
			Ref<Scripting::Script> onPressScript = Assets::AssetService::GetScript(selectionData.m_FunctionPointers.m_OnPressHandle);
			if (!onPressScript)
			{
				KG_WARN("Unable to locate OnPress Script!");
				return;
			}
			selectionData.m_FunctionPointers.m_OnPress = onPressScript;
		}
	}

	void DeserializeImageData(RuntimeUI::ImageData& imageData, YAML::Node& node, const std::string& title)
	{
		imageData.m_ImageHandle = node[(title + "Image")].as<uint64_t>();
		if (imageData.m_ImageHandle == Assets::EmptyHandle)
		{
			imageData.m_ImageRef = nullptr;
		}
		else
		{
			Ref<Rendering::Texture2D> imageRef = Assets::AssetService::GetTexture2D(imageData.m_ImageHandle);
			if (!imageRef)
			{
				KG_WARN("Unable to locate provided image reference");
				return;
			}
			imageData.m_ImageRef = imageRef;
		}
		imageData.m_FixedAspectRatio = node[(title + "FixedAspectRatio")].as<bool>();
	}

	Ref<RuntimeUI::Widget> DeserializeTextWidget(const YAML::Node& node)
	{	
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::TextWidget>();
		YAML::Node specificWidget = node["TextWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::TextWidget;
		RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(widget.get());
		// Get multiline data
		DeserializeMultiLineTextData(textWidget->m_TextData, specificWidget);
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeButtonWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::ButtonWidget>();
		YAML::Node specificWidget = node["ButtonWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::ButtonWidget;
		RuntimeUI::ButtonWidget* buttonWidget = static_cast<RuntimeUI::ButtonWidget*>(widget.get());
		// Get single line data
		DeserializeSingleLineTextData(buttonWidget->m_TextData, specificWidget);
		// Get selection data
		DeserializeSelectionData(buttonWidget->m_SelectionData, specificWidget);
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeImageWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::ImageWidget>();
		YAML::Node specificWidget = node["ImageWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::ImageWidget;
		RuntimeUI::ImageWidget* imageWidget = static_cast<RuntimeUI::ImageWidget*>(widget.get());
		// Get image data
		DeserializeImageData(imageWidget->m_ImageData, specificWidget, "");
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeImageButtonWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::ImageButtonWidget>();
		YAML::Node specificWidget = node["ImageButtonWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::ImageButtonWidget;
		RuntimeUI::ImageButtonWidget* imageButtonWidget = static_cast<RuntimeUI::ImageButtonWidget*>(widget.get());
		// Get selection data
		DeserializeSelectionData(imageButtonWidget->m_SelectionData, specificWidget);
		// Get image data
		DeserializeImageData(imageButtonWidget->m_ImageData, specificWidget, "");
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeCheckboxWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::CheckboxWidget>();
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

	Ref<RuntimeUI::Widget> DeserializeContainerWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::ContainerWidget>();
		YAML::Node specificWidget = node["ContainerWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::ContainerWidget;
		RuntimeUI::ContainerWidget* containerWidget = static_cast<RuntimeUI::ContainerWidget*>(widget.get());
		// Get selection data
		DeserializeContainerData(containerWidget->m_ContainerData, specificWidget);
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeHorizontalContainerWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::HorizontalContainerWidget>();
		YAML::Node specificWidget = node["HorizontalContainerWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::HorizontalContainerWidget;
		RuntimeUI::HorizontalContainerWidget* HorizontalContainerWidget = static_cast<RuntimeUI::HorizontalContainerWidget*>(widget.get());
		// Get unique data
		HorizontalContainerWidget->m_ColumnWidth = specificWidget["ColumnWidth"].as<float>();
		HorizontalContainerWidget->m_ColumnSpacing = specificWidget["ColumnSpacing"].as<float>();
		// Get container data
		DeserializeContainerData(HorizontalContainerWidget->m_ContainerData, specificWidget);
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeVerticalContainerWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::VerticalContainerWidget>();
		YAML::Node specificWidget = node["VerticalContainerWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::VerticalContainerWidget;
		RuntimeUI::VerticalContainerWidget* verticalContainerWidget = static_cast<RuntimeUI::VerticalContainerWidget*>(widget.get());
		// Get unique data
		verticalContainerWidget->m_RowHeight = specificWidget["RowHeight"].as<float>();
		verticalContainerWidget->m_RowSpacing = specificWidget["RowSpacing"].as<float>();
		// Get container data
		DeserializeContainerData(verticalContainerWidget->m_ContainerData, specificWidget);
		return widget;
	}

	Ref<RuntimeUI::Widget> DeserializeInputTextWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::InputTextWidget>();
		YAML::Node specificWidget = node["InputTextWidget"];
		widget->m_WidgetType = RuntimeUI::WidgetTypes::InputTextWidget;
		RuntimeUI::InputTextWidget* inputTextWidget = static_cast<RuntimeUI::InputTextWidget*>(widget.get());
		// Get single line data
		DeserializeSingleLineTextData(inputTextWidget->m_TextData, specificWidget);
		// Get selection data
		DeserializeSelectionData(inputTextWidget->m_SelectionData, specificWidget);
		// Get input map specific function pointers
		inputTextWidget->m_OnMoveCursorHandle = specificWidget["OnMoveCursor"].as<uint64_t>();
		if (inputTextWidget->m_OnMoveCursorHandle == Assets::EmptyHandle)
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

	Ref<RuntimeUI::Widget> DeserializeSliderWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::SliderWidget>();
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
		if (sliderWidget->m_OnMoveSliderHandle == Assets::EmptyHandle)
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

	Ref<RuntimeUI::Widget> DeserializeDropDownWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget = CreateRef<RuntimeUI::DropDownWidget>();
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
		if (dropDownWidget->m_OnSelectOptionHandle == Assets::EmptyHandle)
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

	Ref<RuntimeUI::Widget> DeserializeWidget(const YAML::Node& node)
	{
		Ref<RuntimeUI::Widget> widget;
		RuntimeUI::WidgetTypes widgetType = Utility::StringToWidgetType(node["WidgetType"].as<std::string>());
		switch (widgetType)
		{
		case RuntimeUI::WidgetTypes::TextWidget:
		{
			widget = DeserializeTextWidget(node);
			break;
		}
		case RuntimeUI::WidgetTypes::ButtonWidget:
		{
			widget = DeserializeButtonWidget(node);
			break;
		}
		case RuntimeUI::WidgetTypes::ImageWidget:
		{
			widget = DeserializeImageWidget(node);
			break;
		}
		case RuntimeUI::WidgetTypes::ImageButtonWidget:
		{
			widget = DeserializeImageButtonWidget(node);
			break;
		}
		case RuntimeUI::WidgetTypes::CheckboxWidget:
		{
			widget = DeserializeCheckboxWidget(node);
			break;
		}
		case RuntimeUI::WidgetTypes::ContainerWidget:
		{
			widget = DeserializeContainerWidget(node);
			break;
		}
		case RuntimeUI::WidgetTypes::HorizontalContainerWidget:
		{
			widget = DeserializeHorizontalContainerWidget(node);
			break;
		}
		case RuntimeUI::WidgetTypes::VerticalContainerWidget:
		{
			widget = DeserializeVerticalContainerWidget(node);
			break;
		}
		case RuntimeUI::WidgetTypes::InputTextWidget:
		{
			widget = DeserializeInputTextWidget(node);
			break;
		}

		case RuntimeUI::WidgetTypes::SliderWidget:
		{
			widget = DeserializeSliderWidget(node);
			break;
		}

		case RuntimeUI::WidgetTypes::DropDownWidget:
		{
			widget = DeserializeDropDownWidget(node);
			break;
		}
		default:
		{
			KG_WARN("Invalid Widget Type in UserInterface Deserialization");
			return nullptr;
		}
		}

		widget->m_Tag = node["Tag"].as<std::string>();
		widget->m_ID = node["ID"].as<int32_t>();
		widget->m_PercentPosition = node["PercentPosition"].as<Math::vec2>();
		widget->m_PixelPosition = node["PixelPosition"].as<Math::ivec2>();
		widget->m_XPositionType = Utility::StringToPixelOrPercent(node["XPositionType"].as<std::string>());
		widget->m_YPositionType = Utility::StringToPixelOrPercent(node["YPositionType"].as<std::string>());
		widget->m_XRelativeOrAbsolute = Utility::StringToRelativeOrAbsolute(node["XRelativeOrAbsolute"].as<std::string>());
		widget->m_YRelativeOrAbsolute = Utility::StringToRelativeOrAbsolute(node["YRelativeOrAbsolute"].as<std::string>());
		widget->m_XConstraint = Utility::StringToConstraint(node["XConstraint"].as<std::string>());
		widget->m_YConstraint = Utility::StringToConstraint(node["YConstraint"].as<std::string>());
		widget->m_SizeType = Utility::StringToPixelOrPercent(node["SizeType"].as<std::string>());
		widget->m_PercentSize = node["PercentSize"].as<Math::vec2>();
		widget->m_PixelSize = node["PixelSize"].as<Math::ivec2>();

		return widget;
	}

	Ref<RuntimeUI::UserInterface> UserInterfaceManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		Ref<RuntimeUI::UserInterface> newUserInterface = CreateRef<RuntimeUI::UserInterface>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		// Get SelectColor
		newUserInterface->m_SelectColor = data["SelectColor"].as<Math::vec4>();
		newUserInterface->m_HoveredColor = data["HoveredColor"].as<Math::vec4>();
		newUserInterface->m_EditingColor = data["EditingColor"].as<Math::vec4>();
		// Function Pointers
		newUserInterface->m_FunctionPointers.m_OnMoveHandle = data["FunctionPointerOnMove"].as<uint64_t>();
		if (newUserInterface->m_FunctionPointers.m_OnMoveHandle == Assets::EmptyHandle)
		{
			newUserInterface->m_FunctionPointers.m_OnMove = nullptr;
		}
		else
		{
			Ref<Scripting::Script> onMoveScript = AssetService::GetScript(newUserInterface->m_FunctionPointers.m_OnMoveHandle);
			if (!onMoveScript)
			{
				KG_WARN("Unable to locate OnMove Script!");
				return nullptr;
			}
			newUserInterface->m_FunctionPointers.m_OnMove = onMoveScript;
		}
		newUserInterface->m_FunctionPointers.m_OnHoverHandle = data["FunctionPointerOnHover"].as<uint64_t>();
		if (newUserInterface->m_FunctionPointers.m_OnHoverHandle == Assets::EmptyHandle)
		{
			newUserInterface->m_FunctionPointers.m_OnHover = nullptr;
		}
		else
		{
			Ref<Scripting::Script> onHoverScript = AssetService::GetScript(newUserInterface->m_FunctionPointers.m_OnHoverHandle);
			if (!onHoverScript)
			{
				KG_WARN("Unable to locate OnHover Script!");
				return nullptr;
			}
			newUserInterface->m_FunctionPointers.m_OnHover = onHoverScript;
		}

		// Get Font
		newUserInterface->m_FontHandle = data["Font"].as<uint64_t>();
		newUserInterface->m_Font = AssetService::GetFont(newUserInterface->m_FontHandle);
		// Get Windows
		YAML::Node windows = data["Windows"];
		if (windows)
		{
			std::vector<RuntimeUI::Window>& newWindowsList = newUserInterface->m_Windows;
			for (YAML::detail::iterator_value window : windows)
			{
				RuntimeUI::Window newWindow{};
				newWindow.m_Tag = window["Tag"].as<std::string>();
				newWindow.m_ID = window["ID"].as<int32_t>();
				newWindow.m_ScreenPosition = window["ScreenPosition"].as<Math::vec3>();
				newWindow.m_Size = window["Size"].as<Math::vec2>();
				newWindow.m_BackgroundColor = window["BackgroundColor"].as<Math::vec4>();
				newWindow.m_DefaultActiveWidget = window["DefaultActiveWidget"].as<int32_t>();

				YAML::Node widgetNodes = window["Widgets"];

				if (widgetNodes)
				{
					std::vector<Ref<RuntimeUI::Widget>>& newWidgetsList = newWindow.m_Widgets;
					for (const YAML::Node& widgetNode : widgetNodes)
					{
						Ref<RuntimeUI::Widget> newWidget = DeserializeWidget(widgetNode);
						newWidgetsList.push_back(newWidget);
					}
				}

				newWindowsList.push_back(newWindow);

			}
		}
		return newUserInterface;
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
				inputTextWidget.m_OnMoveCursorHandle = Assets::EmptyHandle;
				inputTextWidget.m_OnMoveCursor = nullptr;
				uiModified = true;
			}
		}

		if (widgetRef->m_WidgetType == RuntimeUI::WidgetTypes::SliderWidget)
		{
			RuntimeUI::SliderWidget& sliderWidget = *(RuntimeUI::SliderWidget*)widgetRef.get();
			if (sliderWidget.m_OnMoveSliderHandle == scriptHandle)
			{
				sliderWidget.m_OnMoveSliderHandle = Assets::EmptyHandle;
				sliderWidget.m_OnMoveSlider = nullptr;
				uiModified = true;
			}
		}

		if (widgetRef->m_WidgetType == RuntimeUI::WidgetTypes::DropDownWidget)
		{
			RuntimeUI::DropDownWidget& dropDownWidget = *(RuntimeUI::DropDownWidget*)widgetRef.get();
			if (dropDownWidget.m_OnSelectOptionHandle == scriptHandle)
			{
				dropDownWidget.m_OnSelectOptionHandle = Assets::EmptyHandle;
				dropDownWidget.m_OnSelectOption = nullptr;
				uiModified = true;
			}
		}

		RuntimeUI::ContainerData* containerData = RuntimeUI::RuntimeUIService::GetContainerDataFromWidget(widgetRef.get());
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
		RuntimeUI::SelectionData* selectionData = RuntimeUI::RuntimeUIService::GetSelectionDataFromWidget(widgetRef.get());
		if (!selectionData)
		{
			return uiModified;
		}

		// Remove script references from widget if necessary
		if (selectionData->m_FunctionPointers.m_OnPressHandle == scriptHandle)
		{
			selectionData->m_FunctionPointers.m_OnPressHandle = Assets::EmptyHandle;
			selectionData->m_FunctionPointers.m_OnPress = nullptr;
			uiModified = true;
		}

		return uiModified;
	}

	bool UserInterfaceManager::RemoveScript(Ref<RuntimeUI::UserInterface> userInterfaceRef, Assets::AssetHandle scriptHandle)
	{
		// Handle UI level function pointers
		bool uiModified{ false };
		if (userInterfaceRef->m_FunctionPointers.m_OnMoveHandle == scriptHandle)
		{
			userInterfaceRef->m_FunctionPointers.m_OnMoveHandle = Assets::EmptyHandle;
			userInterfaceRef->m_FunctionPointers.m_OnMove = nullptr;
			uiModified = true;
		}
		if (userInterfaceRef->m_FunctionPointers.m_OnHoverHandle == scriptHandle)
		{
			userInterfaceRef->m_FunctionPointers.m_OnHoverHandle = Assets::EmptyHandle;
			userInterfaceRef->m_FunctionPointers.m_OnHover = nullptr;
			uiModified = true;
		}

		// Handle all widgets in all windows
		for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_Windows)
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
				buttonWidget.m_ImageData.m_ImageHandle = Assets::EmptyHandle;
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
				imageButtonWidget.m_ImageData.m_ImageHandle = Assets::EmptyHandle;
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
				checkboxWidget.m_ImageUnChecked.m_ImageHandle = Assets::EmptyHandle;
				checkboxWidget.m_ImageUnChecked.m_ImageRef = nullptr;
				uiModified = true;
			}
			if (checkboxWidget.m_ImageChecked.m_ImageHandle == textureHandle)
			{
				checkboxWidget.m_ImageChecked.m_ImageHandle = Assets::EmptyHandle;
				checkboxWidget.m_ImageChecked.m_ImageRef = nullptr;
				uiModified = true;
			}
		}

		RuntimeUI::ContainerData* containerData = RuntimeUI::RuntimeUIService::GetContainerDataFromWidget(widgetRef.get());
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
		for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_Windows)
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
