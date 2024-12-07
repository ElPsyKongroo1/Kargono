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
				out << YAML::Key << "WindowPosition" << YAML::Value << widget->m_WindowPosition;
				out << YAML::Key << "Size" << YAML::Value << widget->m_Size;
				out << YAML::Key << "DefaultBackgroundColor" << YAML::Value << widget->m_DefaultBackgroundColor;
				out << YAML::Key << "WidgetType" << YAML::Value << Utility::WidgetTypeToString(widget->m_WidgetType);
				out << YAML::Key << "Selectable" << YAML::Value << widget->m_Selectable;
				out << YAML::Key << "DirectionPointerUp" << YAML::Value << widget->m_NavigationLinks.m_UpWidgetIndex;
				out << YAML::Key << "DirectionPointerDown" << YAML::Value << widget->m_NavigationLinks.m_DownWidgetIndex;
				out << YAML::Key << "DirectionPointerLeft" << YAML::Value << widget->m_NavigationLinks.m_LeftWidgetIndex;
				out << YAML::Key << "DirectionPointerRight" << YAML::Value << widget->m_NavigationLinks.m_RightWidgetIndex;

				out << YAML::Key << "FunctionPointerOnPress" << YAML::Value << (uint64_t)widget->m_FunctionPointers.m_OnPressHandle;
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
					out << YAML::Key << "TextAbsoluteDimensions" << YAML::Value << textWidget->m_TextAbsoluteDimensions;
					out << YAML::Key << "TextCentered" << YAML::Value << textWidget->m_TextCentered;
					out << YAML::EndMap; // End TextWidget Map
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
							textWidget->m_TextAbsoluteDimensions = specificWidget["TextAbsoluteDimensions"].as<Math::vec2>();
							textWidget->m_TextCentered = specificWidget["TextCentered"].as<bool>();
							break;
						}
						default:
						{
							KG_WARN("Invalid Widget Type in UserInterface Deserialization");
							return nullptr;
						}
						}

						newWidget->m_Tag = widget["Tag"].as<std::string>();
						newWidget->m_WindowPosition = widget["WindowPosition"].as<Math::vec2>();
						newWidget->m_Size = widget["Size"].as<Math::vec2>();
						newWidget->m_DefaultBackgroundColor = widget["DefaultBackgroundColor"].as<Math::vec4>();
						newWidget->m_ActiveBackgroundColor = newWidget->m_DefaultBackgroundColor;
						newWidget->m_Selectable = widget["Selectable"].as<bool>();
						newWidget->m_NavigationLinks.m_UpWidgetIndex = widget["DirectionPointerUp"].as<size_t>();
						newWidget->m_NavigationLinks.m_DownWidgetIndex = widget["DirectionPointerDown"].as<size_t>();
						newWidget->m_NavigationLinks.m_LeftWidgetIndex = widget["DirectionPointerLeft"].as<size_t>();
						newWidget->m_NavigationLinks.m_RightWidgetIndex = widget["DirectionPointerRight"].as<size_t>();

						newWidget->m_FunctionPointers.m_OnPressHandle = widget["FunctionPointerOnPress"].as<uint64_t>();
						if (newWidget->m_FunctionPointers.m_OnPressHandle == Assets::EmptyHandle)
						{
							newWidget->m_FunctionPointers.m_OnPress = nullptr;
						}
						else
						{
							Ref<Scripting::Script> onPressScript = Assets::AssetService::GetScript(newWidget->m_FunctionPointers.m_OnPressHandle);
							if (!onPressScript)
							{
								KG_WARN("Unable to locate OnPress Script!");
								return nullptr;
							}
							newWidget->m_FunctionPointers.m_OnPress = onPressScript;
						}

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

		// Handle all widgets
		for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_Windows)
		{
			for (Ref<RuntimeUI::Widget> widgetRef : currentWindow.m_Widgets)
			{
				if (widgetRef->m_FunctionPointers.m_OnPressHandle == scriptHandle)
				{
					widgetRef->m_FunctionPointers.m_OnPressHandle = Assets::EmptyHandle;
					widgetRef->m_FunctionPointers.m_OnPress = nullptr;
					uiModified = true;
				}
			}
		}

		return uiModified;
	}
}
