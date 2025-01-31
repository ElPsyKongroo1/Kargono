#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/UserInterfaceManager.h"

#include "Kargono/RuntimeUI/RuntimeUI.h"

namespace Kargono::Assets
{
	void UserInterfaceManager::CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		// Create Temporary UserInterface
		Ref<RuntimeUI::UserInterface> temporaryUserInterface = CreateRef<RuntimeUI::UserInterface>();

		// Save into File
		SerializeAsset(temporaryUserInterface, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::UserInterfaceMetaData> metadata = CreateRef<Assets::UserInterfaceMetaData>();
		asset.Data.SpecificFileData = metadata;
	}
	void UserInterfaceManager::SerializeAsset(Ref<RuntimeUI::UserInterface> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Select Color
		out << YAML::Key << "SelectColor" << YAML::Value << assetReference->m_SelectColor;

		// Function Pointers
		out << YAML::Key << "FunctionPointerOnMove" << YAML::Value << (uint64_t)assetReference->m_FunctionPointers.m_OnMoveHandle;
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
				out << YAML::Key << "XPositionType" << YAML::Value << Utility::PixelOrPercentToString(widget->m_XPositionType);
				out << YAML::Key << "YPositionType" << YAML::Value << Utility::PixelOrPercentToString(widget->m_YPositionType);
				out << YAML::Key << "Size" << YAML::Value << widget->m_Size;
				out << YAML::Key << "WidgetType" << YAML::Value << Utility::WidgetTypeToString(widget->m_WidgetType);
				switch (widget->m_WidgetType)
				{
				case RuntimeUI::WidgetTypes::TextWidget:
				{
					RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(widget.get());
					out << YAML::Key << "TextWidget" << YAML::Value;
					out << YAML::BeginMap; // Begin TextWidget Map
					out << YAML::Key << "Text" << YAML::Value << textWidget->m_Text;
					out << YAML::Key << "TextSize" << YAML::Value << textWidget->m_TextSize;
					out << YAML::Key << "TextColor" << YAML::Value << textWidget->m_TextColor;
					out << YAML::Key << "TextAlignment" << YAML::Value << Utility::ConstraintToString(textWidget->m_TextAlignment);
					out << YAML::Key << "TextWrapped" << YAML::Value << textWidget->m_TextWrapped;
					out << YAML::EndMap; // End TextWidget Map
					break;
				}
				case RuntimeUI::WidgetTypes::ButtonWidget:
				{
					RuntimeUI::ButtonWidget* buttonWidget = static_cast<RuntimeUI::ButtonWidget*>(widget.get());
					out << YAML::Key << "ButtonWidget" << YAML::Value;
					out << YAML::BeginMap; // Begin buttonWidget Map
					// Text fields
					out << YAML::Key << "Text" << YAML::Value << buttonWidget->m_Text;
					out << YAML::Key << "TextSize" << YAML::Value << buttonWidget->m_TextSize;
					out << YAML::Key << "TextColor" << YAML::Value << buttonWidget->m_TextColor;
					out << YAML::Key << "TextAlignment" << YAML::Value << Utility::ConstraintToString(buttonWidget->m_TextAlignment);
					// Color fields
					out << YAML::Key << "DefaultBackgroundColor" << YAML::Value << buttonWidget->m_SelectionData.m_DefaultBackgroundColor;
					// Selectable fields
					out << YAML::Key << "Selectable" << YAML::Value << buttonWidget->m_SelectionData.m_Selectable;
					// Direction index fields
					out << YAML::Key << "DirectionPointerUp" << YAML::Value << buttonWidget->m_SelectionData.m_NavigationLinks.m_UpWidgetIndex;
					out << YAML::Key << "DirectionPointerDown" << YAML::Value << buttonWidget->m_SelectionData.m_NavigationLinks.m_DownWidgetIndex;
					out << YAML::Key << "DirectionPointerLeft" << YAML::Value << buttonWidget->m_SelectionData.m_NavigationLinks.m_LeftWidgetIndex;
					out << YAML::Key << "DirectionPointerRight" << YAML::Value << buttonWidget->m_SelectionData.m_NavigationLinks.m_RightWidgetIndex;
					// Function pointer fields
					out << YAML::Key << "FunctionPointerOnPress" << YAML::Value << (uint64_t)buttonWidget->m_SelectionData.m_FunctionPointers.m_OnPressHandle;
					out << YAML::EndMap; // End buttonWidget Map
					break;
				}
				case RuntimeUI::WidgetTypes::ImageWidget:
				{
					RuntimeUI::ImageWidget* imageWidget = static_cast<RuntimeUI::ImageWidget*>(widget.get());
					out << YAML::Key << "ImageWidget" << YAML::Value;
					// Image field
					out << YAML::BeginMap; // Begin ImageWidget Map
					out << YAML::Key << "Image" << YAML::Value << (uint64_t)imageWidget->m_ImageHandle;
					out << YAML::EndMap; // End ImageWidget Map
					break;
				}

				case RuntimeUI::WidgetTypes::ImageButtonWidget:
				{
					RuntimeUI::ImageButtonWidget* imageButtonWidget = static_cast<RuntimeUI::ImageButtonWidget*>(widget.get());
					out << YAML::Key << "ImageButtonWidget" << YAML::Value;
					// Image field
					out << YAML::BeginMap; // Begin ImageWidget Map
					out << YAML::Key << "Image" << YAML::Value << (uint64_t)imageButtonWidget->m_ImageHandle;
					// Color fields
					out << YAML::Key << "DefaultBackgroundColor" << YAML::Value << imageButtonWidget->m_SelectionData.m_DefaultBackgroundColor;
					// Selectable fields
					out << YAML::Key << "Selectable" << YAML::Value << imageButtonWidget->m_SelectionData.m_Selectable;
					// Direction index fields
					out << YAML::Key << "DirectionPointerUp" << YAML::Value << imageButtonWidget->m_SelectionData.m_NavigationLinks.m_UpWidgetIndex;
					out << YAML::Key << "DirectionPointerDown" << YAML::Value << imageButtonWidget->m_SelectionData.m_NavigationLinks.m_DownWidgetIndex;
					out << YAML::Key << "DirectionPointerLeft" << YAML::Value << imageButtonWidget->m_SelectionData.m_NavigationLinks.m_LeftWidgetIndex;
					out << YAML::Key << "DirectionPointerRight" << YAML::Value << imageButtonWidget->m_SelectionData.m_NavigationLinks.m_RightWidgetIndex;
					// Function pointer fields
					out << YAML::Key << "FunctionPointerOnPress" << YAML::Value << (uint64_t)imageButtonWidget->m_SelectionData.m_FunctionPointers.m_OnPressHandle;
					out << YAML::EndMap; // End ImageWidget Map
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
	Ref<RuntimeUI::UserInterface> UserInterfaceManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
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
							textWidget->m_Text = specificWidget["Text"].as<std::string>();
							textWidget->m_TextSize = specificWidget["TextSize"].as<float>();
							textWidget->m_TextColor = specificWidget["TextColor"].as<glm::vec4>();
							textWidget->m_TextAlignment = Utility::StringToConstraint(specificWidget["TextAlignment"].as<std::string>());
							textWidget->m_TextWrapped = specificWidget["TextWrapped"].as<bool>();
							break;
						}
						case RuntimeUI::WidgetTypes::ButtonWidget:
						{
							specificWidget = widget["ButtonWidget"];
							newWidget = CreateRef<RuntimeUI::ButtonWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::ButtonWidget* buttonWidget = static_cast<RuntimeUI::ButtonWidget*>(newWidget.get());
							// Text fields
							buttonWidget->m_Text = specificWidget["Text"].as<std::string>();
							buttonWidget->m_TextSize = specificWidget["TextSize"].as<float>();
							buttonWidget->m_TextColor = specificWidget["TextColor"].as<glm::vec4>();
							buttonWidget->m_TextAlignment = Utility::StringToConstraint(specificWidget["TextAlignment"].as<std::string>());
							// Color fields
							buttonWidget->m_SelectionData.m_DefaultBackgroundColor = specificWidget["DefaultBackgroundColor"].as<Math::vec4>();
							buttonWidget->m_SelectionData.m_ActiveBackgroundColor = buttonWidget->m_SelectionData.m_DefaultBackgroundColor;
							// Selectable field
							buttonWidget->m_SelectionData.m_Selectable = specificWidget["Selectable"].as<bool>();
							// Navigation fields
							buttonWidget->m_SelectionData.m_NavigationLinks.m_UpWidgetIndex = specificWidget["DirectionPointerUp"].as<size_t>();
							buttonWidget->m_SelectionData.m_NavigationLinks.m_DownWidgetIndex = specificWidget["DirectionPointerDown"].as<size_t>();
							buttonWidget->m_SelectionData.m_NavigationLinks.m_LeftWidgetIndex = specificWidget["DirectionPointerLeft"].as<size_t>();
							buttonWidget->m_SelectionData.m_NavigationLinks.m_RightWidgetIndex = specificWidget["DirectionPointerRight"].as<size_t>();
							// Function pointer fields
							buttonWidget->m_SelectionData.m_FunctionPointers.m_OnPressHandle = specificWidget["FunctionPointerOnPress"].as<uint64_t>();
							if (buttonWidget->m_SelectionData.m_FunctionPointers.m_OnPressHandle == Assets::EmptyHandle)
							{
								buttonWidget->m_SelectionData.m_FunctionPointers.m_OnPress = nullptr;
							}
							else
							{
								Ref<Scripting::Script> onPressScript = Assets::AssetService::GetScript(buttonWidget->m_SelectionData.m_FunctionPointers.m_OnPressHandle);
								if (!onPressScript)
								{
									KG_WARN("Unable to locate OnPress Script!");
									return nullptr;
								}
								buttonWidget->m_SelectionData.m_FunctionPointers.m_OnPress = onPressScript;
							}
							break;
						}
						case RuntimeUI::WidgetTypes::ImageWidget:
						{
							specificWidget = widget["ImageWidget"];
							newWidget = CreateRef<RuntimeUI::ImageWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::ImageWidget* imageWidget = static_cast<RuntimeUI::ImageWidget*>(newWidget.get());
							imageWidget->m_ImageHandle = specificWidget["Image"].as<uint64_t>();
							if (imageWidget->m_ImageHandle == Assets::EmptyHandle)
							{
								imageWidget->m_ImageRef = nullptr;
							}
							else
							{
								Ref<Rendering::Texture2D> imageRef = Assets::AssetService::GetTexture2D(imageWidget->m_ImageHandle);
								if (!imageRef)
								{
									KG_WARN("Unable to locate provided image reference");
									return nullptr;
								}
								imageWidget->m_ImageRef = imageRef;
							}
							break;
						}
						case RuntimeUI::WidgetTypes::ImageButtonWidget:
						{
							specificWidget = widget["ImageButtonWidget"];
							newWidget = CreateRef<RuntimeUI::ImageButtonWidget>();
							newWidget->m_WidgetType = widgetType;
							RuntimeUI::ImageButtonWidget* imageButtonWidget = static_cast<RuntimeUI::ImageButtonWidget*>(newWidget.get());
							// Color fields
							imageButtonWidget->m_SelectionData.m_DefaultBackgroundColor = specificWidget["DefaultBackgroundColor"].as<Math::vec4>();
							imageButtonWidget->m_SelectionData.m_ActiveBackgroundColor = imageButtonWidget->m_SelectionData.m_DefaultBackgroundColor;
							// Selectable field
							imageButtonWidget->m_SelectionData.m_Selectable = specificWidget["Selectable"].as<bool>();
							// Navigation fields
							imageButtonWidget->m_SelectionData.m_NavigationLinks.m_UpWidgetIndex = specificWidget["DirectionPointerUp"].as<size_t>();
							imageButtonWidget->m_SelectionData.m_NavigationLinks.m_DownWidgetIndex = specificWidget["DirectionPointerDown"].as<size_t>();
							imageButtonWidget->m_SelectionData.m_NavigationLinks.m_LeftWidgetIndex = specificWidget["DirectionPointerLeft"].as<size_t>();
							imageButtonWidget->m_SelectionData.m_NavigationLinks.m_RightWidgetIndex = specificWidget["DirectionPointerRight"].as<size_t>();
							// Function pointer fields
							imageButtonWidget->m_SelectionData.m_FunctionPointers.m_OnPressHandle = specificWidget["FunctionPointerOnPress"].as<uint64_t>();
							if (imageButtonWidget->m_SelectionData.m_FunctionPointers.m_OnPressHandle == Assets::EmptyHandle)
							{
								imageButtonWidget->m_SelectionData.m_FunctionPointers.m_OnPress = nullptr;
							}
							else
							{
								Ref<Scripting::Script> onPressScript = Assets::AssetService::GetScript(imageButtonWidget->m_SelectionData.m_FunctionPointers.m_OnPressHandle);
								if (!onPressScript)
								{
									KG_WARN("Unable to locate OnPress Script!");
									return nullptr;
								}
								imageButtonWidget->m_SelectionData.m_FunctionPointers.m_OnPress = onPressScript;
							}
							imageButtonWidget->m_ImageHandle = specificWidget["Image"].as<uint64_t>();
							if (imageButtonWidget->m_ImageHandle == Assets::EmptyHandle)
							{
								imageButtonWidget->m_ImageRef = nullptr;
							}
							else
							{
								Ref<Rendering::Texture2D> imageButtonRef = Assets::AssetService::GetTexture2D(imageButtonWidget->m_ImageHandle);
								if (!imageButtonRef)
								{
									KG_WARN("Unable to locate provided ImageButton reference");
									return nullptr;
								}
								imageButtonWidget->m_ImageRef = imageButtonRef;
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
						newWidget->m_Size = widget["Size"].as<Math::vec2>();
						newWidgetsList.push_back(newWidget);


					}
					if (newWindow.m_DefaultActiveWidget != -1) { newWindow.m_DefaultActiveWidgetRef = newWidgetsList.at(newWindow.m_DefaultActiveWidget); }
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

		// Handle all widgets in all windows
		for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_Windows)
		{
			for (Ref<RuntimeUI::Widget> widgetRef : currentWindow.m_Widgets)
			{
				// Ensure we are handling a button widget
				if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::ButtonWidget)
				{
					continue;
				}
				// Remove script references from button widget if necessary
				RuntimeUI::ButtonWidget& buttonWidget = *(RuntimeUI::ButtonWidget*)widgetRef.get();
				if (buttonWidget.m_SelectionData.m_FunctionPointers.m_OnPressHandle == scriptHandle)
				{
					RuntimeUI::ButtonWidget& buttonWidget = *(RuntimeUI::ButtonWidget*)widgetRef.get();
					buttonWidget.m_SelectionData.m_FunctionPointers.m_OnPressHandle = Assets::EmptyHandle;
					buttonWidget.m_SelectionData.m_FunctionPointers.m_OnPress = nullptr;
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
				if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::ImageWidget)
				{
					// Remove texture reference from widget if necessary
					RuntimeUI::ImageWidget& imageWidget = *(RuntimeUI::ImageWidget*)widgetRef.get();
					if (imageWidget.m_ImageHandle == textureHandle)
					{
						RuntimeUI::ImageWidget& buttonWidget = *(RuntimeUI::ImageWidget*)widgetRef.get();
						buttonWidget.m_ImageHandle = Assets::EmptyHandle;
						buttonWidget.m_ImageRef = nullptr;
						uiModified = true;
					}
				}
				if (widgetRef->m_WidgetType != RuntimeUI::WidgetTypes::ImageButtonWidget)
				{
					// Remove texture reference from widget if necessary
					RuntimeUI::ImageButtonWidget& imageWidget = *(RuntimeUI::ImageButtonWidget*)widgetRef.get();
					if (imageWidget.m_ImageHandle == textureHandle)
					{
						RuntimeUI::ImageButtonWidget& imageButtonWidget = *(RuntimeUI::ImageButtonWidget*)widgetRef.get();
						imageButtonWidget.m_ImageHandle = Assets::EmptyHandle;
						imageButtonWidget.m_ImageRef = nullptr;
						uiModified = true;
					}
				}

				
			}
		}

		return uiModified;
	}
}
