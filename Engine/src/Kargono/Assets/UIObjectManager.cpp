#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "API/Serialization/SerializationAPI.h"


namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_UIObjectRegistry {};

	void AssetManager::DeserializeUIObjectRegistry()
	{
		// Clear current registry and open registry in current project 
		s_UIObjectRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& uiObjectRegistryLocation = Projects::Project::GetAssetDirectory() / "UserInterface/UIObjectRegistry.kgreg";

		if (!std::filesystem::exists(uiObjectRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(uiObjectRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kguiObject file '{0}'\n     {1}", uiObjectRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing UIObject Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving uiobject specific metadata 
				if (newAsset.Data.Type == Assets::UIObject)
				{
					Ref<Assets::UIObjectMetaData> uiObjectMetaData = CreateRef<Assets::UIObjectMetaData>();
					newAsset.Data.SpecificFileData = uiObjectMetaData;
				}

				// Add asset to in memory registry 
				s_UIObjectRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeUIObjectRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& uiObjectRegistryLocation = Projects::Project::GetAssetDirectory() / "UserInterface/UIObjectRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "UserInterface";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_UIObjectRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(uiObjectRegistryLocation.parent_path());

		std::ofstream fout(uiObjectRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeUIObject(Ref<RuntimeUI::UIObject> uiObject, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Select Color
		out << YAML::Key << "SelectColor" << YAML::Value << uiObject->m_SelectColor;

		// Function Pointers
		out << YAML::Key << "FunctionPointerOnMove" << YAML::Value << uiObject->m_FunctionPointers.OnMove;
		// Font
		out << YAML::Key << "Font" << YAML::Value << static_cast<uint64_t>(uiObject->m_FontHandle);
		// Windows
		out << YAML::Key << "Windows" << YAML::Value;
		out << YAML::BeginSeq; // Start of Windows Seq

		for (auto& window : uiObject->Windows)
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

			out << YAML::Key << "WidgetCounts" << YAML::Value;
			out << YAML::BeginMap; // Begin WidgetCounts Map
			out << YAML::Key << "TextWidgetCount" << YAML::Value << window.WidgetCounts.TextWidgetCount;
			out << YAML::Key << "TextWidgetLocation" << YAML::Value << window.WidgetCounts.TextWidgetLocation;
			out << YAML::Key << "ButtonWidgetCount" << YAML::Value << window.WidgetCounts.ButtonWidgetCount;
			out << YAML::Key << "ButtonWidgetLocation" << YAML::Value << window.WidgetCounts.ButtonWidgetLocation;
			out << YAML::Key << "CheckboxWidgetCount" << YAML::Value << window.WidgetCounts.CheckboxWidgetCount;
			out << YAML::Key << "CheckboxWidgetLocation" << YAML::Value << window.WidgetCounts.CheckboxWidgetLocation;
			out << YAML::Key << "ComboWidgetCount" << YAML::Value << window.WidgetCounts.ComboWidgetCount;
			out << YAML::Key << "ComboWidgetLocation" << YAML::Value << window.WidgetCounts.ComboWidgetLocation;
			out << YAML::Key << "PopupWidgetCount" << YAML::Value << window.WidgetCounts.PopupWidgetCount;
			out << YAML::Key << "PopupWidgetLocation" << YAML::Value << window.WidgetCounts.PopupWidgetLocation;
			out << YAML::EndMap; // End WidgetCounts Map

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

				out << YAML::Key << "FunctionPointerOnPress" << YAML::Value << widget->FunctionPointers.OnPress;
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

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized UIObject at {}", filepath);
	}

	bool AssetManager::CheckUIObjectExists(const std::string& uiObjectName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(uiObjectName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeUIObject(Ref<RuntimeUI::UIObject> uiObject, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing user interface object");

		// Get SelectColor
		uiObject->m_SelectColor = data["SelectColor"].as<Math::vec4>();
		// Function Pointers
		uiObject->m_FunctionPointers.OnMove = data["FunctionPointerOnMove"].as<std::string>();
		// Get Font
		uiObject->m_FontHandle = data["Font"].as<uint64_t>();
		uiObject->m_Font = AssetManager::GetFont(uiObject->m_FontHandle);
		// Get Windows
		auto windows = data["Windows"];
		if (windows)
		{
			auto& newWindowsList = uiObject->Windows;
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

				auto widgetCounts = window["WidgetCounts"];
				newWindow.WidgetCounts.TextWidgetCount = widgetCounts["TextWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.TextWidgetLocation = widgetCounts["TextWidgetLocation"].as<uint16_t>();
				newWindow.WidgetCounts.ButtonWidgetCount = widgetCounts["ButtonWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.ButtonWidgetLocation = widgetCounts["ButtonWidgetLocation"].as<uint16_t>();
				newWindow.WidgetCounts.CheckboxWidgetCount = widgetCounts["CheckboxWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.CheckboxWidgetLocation = widgetCounts["CheckboxWidgetLocation"].as<uint16_t>();
				newWindow.WidgetCounts.ComboWidgetCount = widgetCounts["ComboWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.ComboWidgetLocation = widgetCounts["ComboWidgetLocation"].as<uint16_t>();
				newWindow.WidgetCounts.PopupWidgetCount = widgetCounts["PopupWidgetCount"].as<uint16_t>();
				newWindow.WidgetCounts.PopupWidgetLocation = widgetCounts["PopupWidgetLocation"].as<uint16_t>();

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
							KG_ASSERT("Invalid Widget Type in RuntimeUI Deserialization");
							return false;
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

						newWidget->FunctionPointers.OnPress = widget["FunctionPointerOnPress"].as<std::string>();

						newWidgetsList.push_back(newWidget);


					}
					if (newWindow.DefaultActiveWidget != -1) { newWindow.DefaultActiveWidgetRef = newWidgetsList.at(newWindow.DefaultActiveWidget); }
				}

				newWindowsList.push_back(newWindow);

			}
		}
		return true;

	}

	AssetHandle AssetManager::CreateNewUIObject(const std::string& uiObjectName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(uiObjectName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateUIObjectFile(uiObjectName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_UIObjectRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeUIObjectRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveUIObject(AssetHandle uiObjectHandle, Ref<RuntimeUI::UIObject> uiObject)
	{
		if (!s_UIObjectRegistry.contains(uiObjectHandle))
		{
			KG_ERROR("Attempt to save uiObject that does not exist in registry");
			return;
		}
		Assets::Asset uiObjectAsset = s_UIObjectRegistry[uiObjectHandle];
		SerializeUIObject(uiObject, (Projects::Project::GetAssetDirectory() / uiObjectAsset.Data.IntermediateLocation).string());
	}

	std::filesystem::path AssetManager::GetUIObjectLocation(const AssetHandle& handle)
	{
		if (!s_UIObjectRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save uiObject that does not exist in registry");
			return "";
		}
		return s_UIObjectRegistry[handle].Data.IntermediateLocation;
	}

	Ref<RuntimeUI::UIObject> AssetManager::GetUIObject(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving uiObject!");

		if (s_UIObjectRegistry.contains(handle))
		{
			auto asset = s_UIObjectRegistry[handle];
			return InstantiateUIObject(asset);
		}

		KG_ERROR("No uiObject is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<RuntimeUI::UIObject>> AssetManager::GetUIObject(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");
		std::filesystem::path uiObjectPath = filepath;

		if (filepath.is_absolute())
		{
			uiObjectPath = Utility::FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_UIObjectRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(uiObjectPath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateUIObject(asset));
			}
		}
		// Return empty uiObject if uiObject does not exist
		KG_WARN("No UIObject Associated with provided handle. Returned new empty uiObject");
		AssetHandle newHandle = CreateNewUIObject(filepath.stem().string());
		return std::make_tuple(newHandle, GetUIObject(newHandle));
	}

	Ref<RuntimeUI::UIObject> AssetManager::InstantiateUIObject(const Assets::Asset& uiObjectAsset)
	{
		Ref<RuntimeUI::UIObject> newUIObject = CreateRef<RuntimeUI::UIObject>();
		DeserializeUIObject(newUIObject, (Projects::Project::GetAssetDirectory() / uiObjectAsset.Data.IntermediateLocation).string());
		return newUIObject;
	}


	void AssetManager::ClearUIObjectRegistry()
	{
		s_UIObjectRegistry.clear();
	}

	void AssetManager::CreateUIObjectFile(const std::string& uiObjectName, Assets::Asset& newAsset)
	{
		// Create Temporary UIObject
		Ref<RuntimeUI::UIObject> temporaryUIObject = CreateRef<RuntimeUI::UIObject>();

		// Save Binary Intermediate into File
		std::string uiObjectPath = "UserInterface/" + uiObjectName + ".kgui";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / uiObjectPath;
		SerializeUIObject(temporaryUIObject, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::UIObject;
		newAsset.Data.IntermediateLocation = uiObjectPath;
		Ref<Assets::UIObjectMetaData> metadata = CreateRef<Assets::UIObjectMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}
}
