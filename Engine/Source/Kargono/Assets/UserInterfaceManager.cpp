#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/UserInterfaceManager.h"

#include "Kargono/RuntimeUI/RuntimeUI.h"

namespace Kargono::Assets
{
	void UserInterfaceManager::CreateAssetFileFromName(const std::string& name, Asset& asset, const std::filesystem::path& assetPath)
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
		out << YAML::Key << "FunctionPointerOnMove" << YAML::Value << (uint64_t)assetReference->m_FunctionPointers.OnMoveHandle;
		// Font
		out << YAML::Key << "Font" << YAML::Value << static_cast<uint64_t>(assetReference->m_FontHandle);
		// Windows
		out << YAML::Key << "Windows" << YAML::Value;
		out << YAML::BeginSeq; // Start of Windows Seq

		for (auto& window : assetReference->m_Windows)
		{
			out << YAML::BeginMap; // Start Window Map

			out << YAML::Key << "Tag" << YAML::Value << window.Tag;
			out << YAML::Key << "ScreenPosition" << YAML::Value << window.ScreenPosition;
			out << YAML::Key << "Size" << YAML::Value << window.Size;
			out << YAML::Key << "BackgroundColor" << YAML::Value << window.BackgroundColor;
			out << YAML::Key << "ParentIndex" << YAML::Value << window.ParentIndex;
			out << YAML::Key << "ChildBufferIndex" << YAML::Value << window.ChildBufferIndex;
			out << YAML::Key << "ChildBufferSize" << YAML::Value << window.ChildBufferSize;
			out << YAML::Key << "DefaultActiveWidget" << YAML::Value << window.DefaultActiveWidget;

			out << YAML::Key << "Widgets" << YAML::Value;
			out << YAML::BeginSeq; // Begin Widget Sequence

			for (auto& widget : window.Widgets)
			{
				out << YAML::BeginMap; // Begin Widget Map

				out << YAML::Key << "Tag" << YAML::Value << widget->Tag;
				out << YAML::Key << "WindowPosition" << YAML::Value << widget->WindowPosition;
				out << YAML::Key << "Size" << YAML::Value << widget->Size;
				out << YAML::Key << "DefaultBackgroundColor" << YAML::Value << widget->DefaultBackgroundColor;
				out << YAML::Key << "WidgetType" << YAML::Value << Utility::WidgetTypeToString(widget->WidgetType);
				out << YAML::Key << "Selectable" << YAML::Value << widget->Selectable;
				out << YAML::Key << "DirectionPointerUp" << YAML::Value << widget->DirectionPointer.Up;
				out << YAML::Key << "DirectionPointerDown" << YAML::Value << widget->DirectionPointer.Down;
				out << YAML::Key << "DirectionPointerLeft" << YAML::Value << widget->DirectionPointer.Left;
				out << YAML::Key << "DirectionPointerRight" << YAML::Value << widget->DirectionPointer.Right;

				out << YAML::Key << "FunctionPointerOnPress" << YAML::Value << (uint64_t)widget->FunctionPointers.OnPressHandle;
				switch (widget->WidgetType)
				{
				case RuntimeUI::WidgetTypes::TextWidget:
				{
					RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(widget.get());
					out << YAML::Key << "TextWidget" << YAML::Value;
					out << YAML::BeginMap; // Begin TextWidget Map
					out << YAML::Key << "Text" << YAML::Value << textWidget->Text;
					out << YAML::Key << "TextSize" << YAML::Value << textWidget->TextSize;
					out << YAML::Key << "TextColor" << YAML::Value << textWidget->TextColor;
					out << YAML::Key << "TextAbsoluteDimensions" << YAML::Value << textWidget->TextAbsoluteDimensions;
					out << YAML::Key << "TextCentered" << YAML::Value << textWidget->TextCentered;
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
	Ref<RuntimeUI::UserInterface> UserInterfaceManager::DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath)
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
		newUserInterface->m_FunctionPointers.OnMoveHandle = data["FunctionPointerOnMove"].as<uint64_t>();
		if (newUserInterface->m_FunctionPointers.OnMoveHandle == Assets::EmptyHandle)
		{
			newUserInterface->m_FunctionPointers.OnMove = nullptr;
		}
		else
		{
			Ref<Scripting::Script> onMoveScript = AssetService::GetScript(newUserInterface->m_FunctionPointers.OnMoveHandle);
			if (!onMoveScript)
			{
				KG_WARN("Unable to locate OnMove Script!");
				return nullptr;
			}
			newUserInterface->m_FunctionPointers.OnMove = onMoveScript;
		}

		// Get Font
		newUserInterface->m_FontHandle = data["Font"].as<uint64_t>();
		newUserInterface->m_Font = AssetService::GetFont(newUserInterface->m_FontHandle);
		// Get Windows
		auto windows = data["Windows"];
		if (windows)
		{
			auto& newWindowsList = newUserInterface->m_Windows;
			for (auto window : windows)
			{
				RuntimeUI::Window newWindow{};
				newWindow.Tag = window["Tag"].as<std::string>();
				newWindow.ScreenPosition = window["ScreenPosition"].as<Math::vec3>();
				newWindow.Size = window["Size"].as<Math::vec2>();
				newWindow.BackgroundColor = window["BackgroundColor"].as<Math::vec4>();
				newWindow.ParentIndex = window["ParentIndex"].as<int32_t>();
				newWindow.ChildBufferIndex = window["ChildBufferIndex"].as<int32_t>();
				newWindow.ChildBufferSize = window["ChildBufferSize"].as<uint32_t>();
				newWindow.DefaultActiveWidget = window["DefaultActiveWidget"].as<int32_t>();

				auto widgets = window["Widgets"];

				if (widgets)
				{
					auto& newWidgetsList = newWindow.Widgets;
					for (auto widget : widgets)
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
							newWidget->WidgetType = widgetType;
							RuntimeUI::TextWidget* textWidget = static_cast<RuntimeUI::TextWidget*>(newWidget.get());
							textWidget->Text = specificWidget["Text"].as<std::string>();
							textWidget->TextSize = specificWidget["TextSize"].as<float>();
							textWidget->TextColor = specificWidget["TextColor"].as<glm::vec4>();
							textWidget->TextAbsoluteDimensions = specificWidget["TextAbsoluteDimensions"].as<Math::vec2>();
							textWidget->TextCentered = specificWidget["TextCentered"].as<bool>();
							break;
						}
						default:
						{
							KG_WARN("Invalid Widget Type in UserInterface Deserialization");
							return nullptr;
						}
						}

						newWidget->Tag = widget["Tag"].as<std::string>();
						newWidget->WindowPosition = widget["WindowPosition"].as<Math::vec2>();
						newWidget->Size = widget["Size"].as<Math::vec2>();
						newWidget->DefaultBackgroundColor = widget["DefaultBackgroundColor"].as<Math::vec4>();
						newWidget->ActiveBackgroundColor = newWidget->DefaultBackgroundColor;
						newWidget->Selectable = widget["Selectable"].as<bool>();
						newWidget->DirectionPointer.Up = widget["DirectionPointerUp"].as<int32_t>();
						newWidget->DirectionPointer.Down = widget["DirectionPointerDown"].as<int32_t>();
						newWidget->DirectionPointer.Left = widget["DirectionPointerLeft"].as<int32_t>();
						newWidget->DirectionPointer.Right = widget["DirectionPointerRight"].as<int32_t>();

						newWidget->FunctionPointers.OnPressHandle = widget["FunctionPointerOnPress"].as<uint64_t>();
						if (newWidget->FunctionPointers.OnPressHandle == Assets::EmptyHandle)
						{
							newWidget->FunctionPointers.OnPress = nullptr;
						}
						else
						{
							Ref<Scripting::Script> onPressScript = Assets::AssetService::GetScript(newWidget->FunctionPointers.OnPressHandle);
							if (!onPressScript)
							{
								KG_WARN("Unable to locate OnPress Script!");
								return nullptr;
							}
							newWidget->FunctionPointers.OnPress = onPressScript;
						}

						newWidgetsList.push_back(newWidget);


					}
					if (newWindow.DefaultActiveWidget != -1) { newWindow.DefaultActiveWidgetRef = newWidgetsList.at(newWindow.DefaultActiveWidget); }
				}

				newWindowsList.push_back(newWindow);

			}
		}
		return newUserInterface;
	}
}
