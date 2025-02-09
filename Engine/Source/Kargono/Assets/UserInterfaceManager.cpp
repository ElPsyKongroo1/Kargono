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

	static void SerializeSelectionData(YAML::Emitter& out, RuntimeUI::SelectionData& selectionData)
	{
		// Color fields
		out << YAML::Key << "DefaultBackgroundColor" << YAML::Value << selectionData.m_DefaultBackgroundColor;
		// Selectable fields
		out << YAML::Key << "Selectable" << YAML::Value << selectionData.m_Selectable;
		// Function pointer fields
		out << YAML::Key << "FunctionPointerOnPress" << YAML::Value << (uint64_t)selectionData.m_FunctionPointers.m_OnPressHandle;
	}
	static void SerializeImageData(YAML::Emitter& out, RuntimeUI::ImageData& imageData, const std::string& title)
	{
		out << YAML::Key << (title + "Image") << YAML::Value << (uint64_t)imageData.m_ImageHandle;
		out << YAML::Key << (title + "FixedAspectRatio") << YAML::Value << imageData.m_FixedAspectRatio;
	}
	static void SerializeSingleLineTextData(YAML::Emitter& out, RuntimeUI::SingleLineTextData& textData)
	{
		out << YAML::Key << "Text" << YAML::Value << textData.m_Text;
		out << YAML::Key << "TextSize" << YAML::Value << textData.m_TextSize;
		out << YAML::Key << "TextColor" << YAML::Value << textData.m_TextColor;
		out << YAML::Key << "TextAlignment" << YAML::Value << Utility::ConstraintToString(textData.m_TextAlignment);
	}
	static void SerializeMultiLineTextData(YAML::Emitter& out, RuntimeUI::MultiLineTextData& textData)
	{
		out << YAML::Key << "Text" << YAML::Value << textData.m_Text;
		out << YAML::Key << "TextSize" << YAML::Value << textData.m_TextSize;
		out << YAML::Key << "TextColor" << YAML::Value << textData.m_TextColor;
		out << YAML::Key << "TextAlignment" << YAML::Value << Utility::ConstraintToString(textData.m_TextAlignment);
		out << YAML::Key << "TextWrapped" << YAML::Value << textData.m_TextWrapped;
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
			out << YAML::Key << "ScreenPosition" << YAML::Value << window.m_ScreenPosition;
			out << YAML::Key << "Size" << YAML::Value << window.m_Size;
			out << YAML::Key << "BackgroundColor" << YAML::Value << window.m_BackgroundColor;
			out << YAML::Key << "ParentIndex" << YAML::Value << window.m_ParentIndex;
			out << YAML::Key << "ChildBufferIndex" << YAML::Value << window.m_ChildBufferIndex;
			out << YAML::Key << "ChildBufferSize" << YAML::Value << window.m_ChildBufferSize;
			out << YAML::Key << "DefaultActiveWidget" << YAML::Value << window.m_DefaultActiveWidget;

			out << YAML::Key << "Widgets" << YAML::Value;
			out << YAML::BeginSeq; // Begin Widget Sequence

			for (Ref<RuntimeUI::Widget> widget : window.m_Widgets)
			{
				out << YAML::BeginMap; // Begin Widget Map

				out << YAML::Key << "Tag" << YAML::Value << widget->m_Tag;
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
					RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(widget.get());
					out << YAML::Key << "TextWidget" << YAML::Value;
					out << YAML::BeginMap; // Begin TextWidget Map
					SerializeMultiLineTextData(out, textWidget->m_TextData);
					out << YAML::EndMap; // End TextWidget Map
					break;
				}
				case RuntimeUI::WidgetTypes::ButtonWidget:
				{
					RuntimeUI::ButtonWidget* buttonWidget = static_cast<RuntimeUI::ButtonWidget*>(widget.get());
					out << YAML::Key << "ButtonWidget" << YAML::Value;
					out << YAML::BeginMap; // Begin buttonWidget Map
					// Save text data
					SerializeSingleLineTextData(out, buttonWidget->m_TextData);
					// Save selection fields
					SerializeSelectionData(out, buttonWidget->m_SelectionData);
					out << YAML::EndMap; // End buttonWidget Map
					break;
				}
				case RuntimeUI::WidgetTypes::ImageWidget:
				{
					RuntimeUI::ImageWidget* imageWidget = static_cast<RuntimeUI::ImageWidget*>(widget.get());
					out << YAML::Key << "ImageWidget" << YAML::Value;
					// Image field
					out << YAML::BeginMap; // Begin ImageWidget Map
					// Save image data
					SerializeImageData(out, imageWidget->m_ImageData, "");
					out << YAML::EndMap; // End ImageWidget Map
					break;
				}

				case RuntimeUI::WidgetTypes::ImageButtonWidget:
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
					break;
				}
				case RuntimeUI::WidgetTypes::CheckboxWidget:
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
					break;
				}

				case RuntimeUI::WidgetTypes::InputTextWidget:
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
					break;
				}

				case RuntimeUI::WidgetTypes::SliderWidget:
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
					break;
				}

				case RuntimeUI::WidgetTypes::DropDownWidget:
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
					break;
				}
				}


				out << YAML::EndMap; // End Widget Map
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

	static void DeserializeMultiLineTextData(RuntimeUI::MultiLineTextData& textData, YAML::Node& node)
	{
		textData.m_Text = node["Text"].as<std::string>();
		textData.m_TextSize = node["TextSize"].as<float>();
		textData.m_TextColor = node["TextColor"].as<glm::vec4>();
		textData.m_TextAlignment = Utility::StringToConstraint(node["TextAlignment"].as<std::string>());
		textData.m_TextWrapped = node["TextWrapped"].as<bool>();
	}
	static void DeserializeSingleLineTextData(RuntimeUI::SingleLineTextData& textData, const YAML::Node& node)
	{
		// Text fields
		textData.m_Text = node["Text"].as<std::string>();
		textData.m_TextSize = node["TextSize"].as<float>();
		textData.m_TextColor = node["TextColor"].as<glm::vec4>();
		textData.m_TextAlignment = Utility::StringToConstraint(node["TextAlignment"].as<std::string>());
	}
	static void DeserializeSelectionData(RuntimeUI::SelectionData& selectionData, YAML::Node& node)
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

	static void DeserializeImageData(RuntimeUI::ImageData& imageData, YAML::Node& node, const std::string& title)
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
				newWindow.m_ScreenPosition = window["ScreenPosition"].as<Math::vec3>();
				newWindow.m_Size = window["Size"].as<Math::vec2>();
				newWindow.m_BackgroundColor = window["BackgroundColor"].as<Math::vec4>();
				newWindow.m_ParentIndex = window["ParentIndex"].as<size_t>();
				newWindow.m_ChildBufferIndex = window["ChildBufferIndex"].as<size_t>();
				newWindow.m_ChildBufferSize = window["ChildBufferSize"].as<size_t>();
				newWindow.m_DefaultActiveWidget = window["DefaultActiveWidget"].as<size_t>();

				YAML::Node widgets = window["Widgets"];

				if (widgets)
				{
					std::vector<Ref<RuntimeUI::Widget>>& newWidgetsList = newWindow.m_Widgets;
					for (YAML::detail::iterator_value widget : widgets)
					{
						RuntimeUI::WidgetTypes widgetType = Utility::StringToWidgetType(widget["WidgetType"].as<std::string>());
						Ref<RuntimeUI::Widget> newWidget = nullptr;
						YAML::Node specificWidget;
						switch (widgetType)
						{
						case RuntimeUI::WidgetTypes::TextWidget:
						{
							specificWidget = widget["TextWidget"];
							newWidget = CreateRef<RuntimeUI::TextWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(newWidget.get());
							// Get multiline data
							DeserializeMultiLineTextData(textWidget->m_TextData, specificWidget);
							break;
						}
						case RuntimeUI::WidgetTypes::ButtonWidget:
						{
							specificWidget = widget["ButtonWidget"];
							newWidget = CreateRef<RuntimeUI::ButtonWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::ButtonWidget* buttonWidget = static_cast<RuntimeUI::ButtonWidget*>(newWidget.get());
							// Get single line data
							DeserializeSingleLineTextData(buttonWidget->m_TextData, specificWidget);
							// Get selection data
							DeserializeSelectionData(buttonWidget->m_SelectionData, specificWidget);
							break;
						}
						case RuntimeUI::WidgetTypes::ImageWidget:
						{
							specificWidget = widget["ImageWidget"];
							newWidget = CreateRef<RuntimeUI::ImageWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::ImageWidget* imageWidget = static_cast<RuntimeUI::ImageWidget*>(newWidget.get());
							// Get image data
							DeserializeImageData(imageWidget->m_ImageData, specificWidget, "");
							break;
						}
						case RuntimeUI::WidgetTypes::ImageButtonWidget:
						{
							specificWidget = widget["ImageButtonWidget"];
							newWidget = CreateRef<RuntimeUI::ImageButtonWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::ImageButtonWidget* imageButtonWidget = static_cast<RuntimeUI::ImageButtonWidget*>(newWidget.get());
							// Get selection data
							DeserializeSelectionData(imageButtonWidget->m_SelectionData, specificWidget);
							// Get image data
							DeserializeImageData(imageButtonWidget->m_ImageData, specificWidget, "");
							break;
						}
						case RuntimeUI::WidgetTypes::CheckboxWidget:
						{
							specificWidget = widget["CheckboxWidget"];
							newWidget = CreateRef<RuntimeUI::CheckboxWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::CheckboxWidget* checkboxWidget = static_cast<RuntimeUI::CheckboxWidget*>(newWidget.get());
							// Get checked status
							checkboxWidget->m_Checked= specificWidget["Checked"].as<bool>();
							// Get selection data
							DeserializeSelectionData(checkboxWidget->m_SelectionData, specificWidget);
							// Get checked image data
							DeserializeImageData(checkboxWidget->m_ImageChecked, specificWidget, "Checked");
							// Get unchecked image data
							DeserializeImageData(checkboxWidget->m_ImageUnChecked, specificWidget, "UnChecked");
							break;
						}
						case RuntimeUI::WidgetTypes::InputTextWidget:
						{
							specificWidget = widget["InputTextWidget"];
							newWidget = CreateRef<RuntimeUI::InputTextWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::InputTextWidget* inputTextWidget = static_cast<RuntimeUI::InputTextWidget*>(newWidget.get());
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
							break;
						}

						case RuntimeUI::WidgetTypes::SliderWidget:
						{
							specificWidget = widget["SliderWidget"];
							newWidget = CreateRef<RuntimeUI::SliderWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::SliderWidget* sliderWidget = static_cast<RuntimeUI::SliderWidget*>(newWidget.get());
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
							break;
						}

						case RuntimeUI::WidgetTypes::DropDownWidget:
						{
							specificWidget = widget["DropDownWidget"];
							newWidget = CreateRef<RuntimeUI::DropDownWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::DropDownWidget* dropDownWidget = static_cast<RuntimeUI::DropDownWidget*>(newWidget.get());
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
							break;
						}
						

						default:
						{
							KG_WARN("Invalid Widget Type in UserInterface Deserialization");
							return nullptr;
						}
						}

						newWidget->m_Tag = widget["Tag"].as<std::string>();
						newWidget->m_PercentPosition = widget["PercentPosition"].as<Math::vec2>();
						newWidget->m_PixelPosition = widget["PixelPosition"].as<Math::ivec2>();
						newWidget->m_XPositionType = Utility::StringToPixelOrPercent(widget["XPositionType"].as<std::string>());
						newWidget->m_YPositionType = Utility::StringToPixelOrPercent(widget["YPositionType"].as<std::string>());
						newWidget->m_XRelativeOrAbsolute = Utility::StringToRelativeOrAbsolute(widget["XRelativeOrAbsolute"].as<std::string>());
						newWidget->m_YRelativeOrAbsolute = Utility::StringToRelativeOrAbsolute(widget["YRelativeOrAbsolute"].as<std::string>());
						newWidget->m_XConstraint = Utility::StringToConstraint(widget["XConstraint"].as<std::string>());
						newWidget->m_YConstraint= Utility::StringToConstraint(widget["YConstraint"].as<std::string>());
						newWidget->m_SizeType = Utility::StringToPixelOrPercent(widget["SizeType"].as<std::string>());
						newWidget->m_PercentSize = widget["PercentSize"].as<Math::vec2>();
						newWidget->m_PixelSize = widget["PixelSize"].as<Math::ivec2>();
						newWidgetsList.push_back(newWidget);
					}
					if (newWindow.m_DefaultActiveWidget != -1) 
					{
						newWindow.m_DefaultActiveWidgetRef = newWidgetsList.at(newWindow.m_DefaultActiveWidget); 
					}
				}

				newWindowsList.push_back(newWindow);

			}
		}
		return newUserInterface;
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
				// Check if this widget has a selection data
				RuntimeUI::SelectionData* selectionData = RuntimeUI::RuntimeUIService::GetSelectionDataFromWidget(widgetRef.get());
				if (!selectionData)
				{
					continue;
				}

				// Remove script references from widget if necessary
				if (selectionData->m_FunctionPointers.m_OnPressHandle == scriptHandle)
				{
					selectionData->m_FunctionPointers.m_OnPressHandle = Assets::EmptyHandle;
					selectionData->m_FunctionPointers.m_OnPress = nullptr;
					uiModified = true;
				}

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

				
			}
		}

		return uiModified;
	}
}
