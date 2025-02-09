#include "kgpch.h"

#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Core/Engine.h"


#if defined(KG_PLATFORM_WINDOWS) 

#include "API/Platform/WindowsBackendAPI.h"
#include "API/Platform/GlfwBackendAPI.h"

namespace Kargono::Utility
{
	std::filesystem::path FileDialogs::OpenFile(const char* filter, const char* initialDirectory)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)EngineService::GetActiveWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (initialDirectory == nullptr || *initialDirectory == '\0')
		{
			if (GetCurrentDirectoryA(256, currentDir))
			{
				ofn.lpstrInitialDir = currentDir;
			}
		}
		else
		{
			ofn.lpstrInitialDir = initialDirectory;
		}

		if (filter != nullptr && *filter != '\0')
		{
			ofn.lpstrFilter = filter;
		}
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();

	}

		CHAR szFile[260] = { 0 };
	std::filesystem::path FileDialogs::SaveFile(const char* filter, const char* initialDirectory)
	{
		OPENFILENAMEA ofn;
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)EngineService::GetActiveWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		if (initialDirectory == nullptr || *initialDirectory == '\0')
		{
			if (GetCurrentDirectoryA(256, currentDir))
			{
				ofn.lpstrInitialDir = currentDir;
			}
		}
		else
		{
			ofn.lpstrInitialDir = initialDirectory;
		}

		if (filter != nullptr && *filter != '\0')
		{
			ofn.lpstrFilter = filter;
		}
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = strchr(filter, '\0') + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

	std::filesystem::path FileDialogs::ChooseDirectory(const std::filesystem::path& initialPath)
	{
		// Initialize COM library
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (FAILED(hr)) 
		{
			KG_WARN("Failed to initialize COM library");
			return {};
		}

		IFileDialog* pFileDialog = nullptr;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileDialog, reinterpret_cast<void**>(&pFileDialog));
		if (FAILED(hr)) 
		{
			KG_WARN("Failed to create FileOpenDialog instance.");
			CoUninitialize();
			return {};
		}

		DWORD dwOptions;
		hr = pFileDialog->GetOptions(&dwOptions);
		if (SUCCEEDED(hr)) 
		{
			// Set the options for the file dialog to select folders
			hr = pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
		}

		if (FAILED(hr)) 
		{
			KG_WARN("Failed to set dialog options.");
			pFileDialog->Release();
			CoUninitialize();
			return {};
		}

		// Set the initial folder
		if (!initialPath.empty()) 
		{
			PIDLIST_ABSOLUTE pidl;
			SFGAOF sfgao;
			hr = SHParseDisplayName(initialPath.c_str(), nullptr, &pidl, 0, &sfgao);
			if (SUCCEEDED(hr)) {
				IShellItem* psiFolder = nullptr;
				hr = SHCreateShellItem(nullptr, nullptr, pidl, &psiFolder);
				if (SUCCEEDED(hr)) {
					pFileDialog->SetFolder(psiFolder);
					psiFolder->Release();
				}
				CoTaskMemFree(pidl);
			}
		}

		// Show the dialog
		hr = pFileDialog->Show(NULL);
		if (FAILED(hr)) 
		{
			KG_WARN("Dialog was canceled or an error occurred.");
			pFileDialog->Release();
			CoUninitialize();
			return {};
		}

		// Get the selected folder
		IShellItem* pItem = nullptr;
		hr = pFileDialog->GetResult(&pItem);
		if (SUCCEEDED(hr)) 
		{
			PWSTR pszFilePath = nullptr;
			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
			if (SUCCEEDED(hr)) {
				std::filesystem::path selectedPath(pszFilePath);
				CoTaskMemFree(pszFilePath);
				pItem->Release();
				pFileDialog->Release();
				CoUninitialize();
				return selectedPath;
			}
			pItem->Release();
		}

		pFileDialog->Release();
		CoUninitialize();
		return {};
	}
}

#endif
