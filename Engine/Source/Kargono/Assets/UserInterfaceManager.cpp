#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"

#include "API/Serialization/yamlcppAPI.h"


namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_UserInterfaceRegistry {};

	void AssetManager::DeserializeUserInterfaceRegistry()
	{
		// Clear current registry and open registry in current project 
		s_UserInterfaceRegistry.clear();
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& userInterfaceRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "UserInterface/UserInterfaceRegistry.kgreg";

		if (!std::filesystem::exists(userInterfaceRegistryLocation))
		{
			KG_WARN("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(userInterfaceRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", userInterfaceRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing UserInterface Registry");

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
				if (newAsset.Data.Type == Assets::AssetType::UserInterface)
				{
					Ref<Assets::UserInterfaceMetaData> userInterfaceMetaData = CreateRef<Assets::UserInterfaceMetaData>();
					newAsset.Data.SpecificFileData = userInterfaceMetaData;
				}

				// Add asset to in memory registry 
				s_UserInterfaceRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeUserInterfaceRegistry()
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& userInterfaceRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "UserInterface/UserInterfaceRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "UserInterface";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_UserInterfaceRegistry)
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

		Utility::FileSystem::CreateNewDirectory(userInterfaceRegistryLocation.parent_path());

		std::ofstream fout(userInterfaceRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeUserInterface(Ref<RuntimeUI::UserInterface> userInterface, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Select Color
		out << YAML::Key << "SelectColor" << YAML::Value << userInterface->m_SelectColor;

		// Function Pointers
		out << YAML::Key << "FunctionPointerOnMove" << YAML::Value << (uint64_t)userInterface->m_FunctionPointers.OnMoveHandle;
		// Font
		out << YAML::Key << "Font" << YAML::Value << static_cast<uint64_t>(userInterface->m_FontHandle);
		// Windows
		out << YAML::Key << "Windows" << YAML::Value;
		out << YAML::BeginSeq; // Start of Windows Seq

		for (auto& window : userInterface->m_Windows)
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

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized UserInterface at {}", filepath);
	}

	bool AssetManager::CheckUserInterfaceExists(const std::string& userInterfaceName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(userInterfaceName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_UserInterfaceRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeUserInterface(Ref<RuntimeUI::UserInterface> userInterface, const std::filesystem::path& filepath)
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
		userInterface->m_SelectColor = data["SelectColor"].as<Math::vec4>();
		// Function Pointers
		userInterface->m_FunctionPointers.OnMoveHandle = data["FunctionPointerOnMove"].as<uint64_t>();
		if (userInterface->m_FunctionPointers.OnMoveHandle == Assets::EmptyHandle)
		{
			userInterface->m_FunctionPointers.OnMove = nullptr;
		}
		else
		{
			Ref<Scripting::Script> onMoveScript = GetScript(userInterface->m_FunctionPointers.OnMoveHandle);
			if (!onMoveScript)
			{
				KG_ERROR("Unable to locate OnMove Script!");
				return false;
			}
			userInterface->m_FunctionPointers.OnMove = onMoveScript;
		}
		
		// Get Font
		userInterface->m_FontHandle = data["Font"].as<uint64_t>();
		userInterface->m_Font = GetFont(userInterface->m_FontHandle);
		// Get Windows
		auto windows = data["Windows"];
		if (windows)
		{
			auto& newWindowsList = userInterface->m_Windows;
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
								return false;
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
		return true;

	}

	AssetHandle AssetManager::CreateNewUserInterface(const std::string& userInterfaceName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(userInterfaceName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_UserInterfaceRegistry)
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
		CreateUserInterfaceFile(userInterfaceName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_UserInterfaceRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeUserInterfaceRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::DeleteUserInterface(AssetHandle handle)
	{
		if (!s_UserInterfaceRegistry.contains(handle))
		{
			KG_WARN("Failed to delete user interface in AssetManager");
			return;
		}

		Utility::FileSystem::DeleteSelectedFile(Projects::ProjectService::GetActiveAssetDirectory() /
			s_UserInterfaceRegistry.at(handle).Data.IntermediateLocation);

		s_UserInterfaceRegistry.erase(handle);

		SerializeUserInterfaceRegistry();
	}

	void AssetManager::SaveUserInterface(AssetHandle userInterfaceHandle, Ref<RuntimeUI::UserInterface> userInterface)
	{
		if (!s_UserInterfaceRegistry.contains(userInterfaceHandle))
		{
			KG_ERROR("Attempt to save userInterface that does not exist in registry");
			return;
		}
		Assets::Asset userInterfaceAsset = s_UserInterfaceRegistry[userInterfaceHandle];
		SerializeUserInterface(userInterface, (Projects::ProjectService::GetActiveAssetDirectory() / userInterfaceAsset.Data.IntermediateLocation).string());
	}

	std::filesystem::path AssetManager::GetUserInterfaceLocation(const AssetHandle& handle)
	{
		if (!s_UserInterfaceRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save userInterface that does not exist in registry");
			return "";
		}
		return s_UserInterfaceRegistry[handle].Data.IntermediateLocation;
	}

	Ref<RuntimeUI::UserInterface> AssetManager::GetUserInterface(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retreiving userInterface!");

		if (s_UserInterfaceRegistry.contains(handle))
		{
			auto asset = s_UserInterfaceRegistry[handle];
			return InstantiateUserInterface(asset);
		}

		KG_ERROR("No userInterface is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<RuntimeUI::UserInterface>> AssetManager::GetUserInterface(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "Attempt to use Project Field without active project!");
		std::filesystem::path userInterfacePath = filepath;

		if (filepath.is_absolute())
		{
			userInterfacePath = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_UserInterfaceRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(userInterfacePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateUserInterface(asset));
			}
		}
		// Return empty userInterface if userInterface does not exist
		KG_WARN("No UserInterface Associated with provided handle. Returned new empty userInterface");
		AssetHandle newHandle = CreateNewUserInterface(filepath.stem().string());
		return std::make_tuple(newHandle, GetUserInterface(newHandle));
	}

	Ref<RuntimeUI::UserInterface> AssetManager::InstantiateUserInterface(const Assets::Asset& userInterfaceAsset)
	{
		Ref<RuntimeUI::UserInterface> newUIObject = CreateRef<RuntimeUI::UserInterface>();
		DeserializeUserInterface(newUIObject, (Projects::ProjectService::GetActiveAssetDirectory() / userInterfaceAsset.Data.IntermediateLocation).string());
		return newUIObject;
	}


	void AssetManager::ClearUserInterfaceRegistry()
	{
		s_UserInterfaceRegistry.clear();
	}

	void AssetManager::CreateUserInterfaceFile(const std::string& userInterfaceName, Assets::Asset& newAsset)
	{
		// Create Temporary UserInterface
		Ref<RuntimeUI::UserInterface> temporaryUserInterface = CreateRef<RuntimeUI::UserInterface>();

		// Save Binary Intermediate into File
		std::string userInterfacePath = "UserInterface/" + userInterfaceName + ".kgui";
		std::filesystem::path intermediateFullPath = Projects::ProjectService::GetActiveAssetDirectory() / userInterfacePath;
		SerializeUserInterface(temporaryUserInterface, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::UserInterface;
		newAsset.Data.IntermediateLocation = userInterfacePath;
		Ref<Assets::UserInterfaceMetaData> metadata = CreateRef<Assets::UserInterfaceMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}
}
