#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/UserInterfaceManagerTemp.h"

#include "Kargono/RuntimeUI/RuntimeUI.h"

namespace Kargono::Assets
{
	Ref<RuntimeUI::UserInterface> UserInterfaceManager::InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Ref<RuntimeUI::UserInterface> newUserInterface = CreateRef<RuntimeUI::UserInterface>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		KG_INFO("Deserializing user interface object");

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
			Ref<Scripting::Script> onMoveScript = AssetServiceTemp::GetScript(newUserInterface->m_FunctionPointers.OnMoveHandle);
			if (!onMoveScript)
			{
				KG_ERROR("Unable to locate OnMove Script!");
				return nullptr;
			}
			newUserInterface->m_FunctionPointers.OnMove = onMoveScript;
		}

		// Get Font
		newUserInterface->m_FontHandle = data["Font"].as<uint64_t>();
		newUserInterface->m_Font = AssetServiceTemp::GetFont(newUserInterface->m_FontHandle);
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
							KG_ASSERT("Invalid Widget Type in UserInterface Deserialization");
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
							Ref<Scripting::Script> onPressScript = Assets::AssetManager::GetScript(newWidget->FunctionPointers.OnPressHandle);
							if (!onPressScript)
							{
								KG_ERROR("Unable to locate OnPress Script!");
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
