#include "kgpch.h"
#include "FileWatchAPI.h"
#include "FileSystemPlugin/FileSystem.h"

namespace API::FileWatch
{
	static std::unordered_map<std::string, filewatch::FileWatch<std::string>*> s_AllFileWatchers {};

	bool StartWatch(const std::filesystem::path& path, std::function<void(const std::string&, const EventType change_type)> func)
	{
		if (s_AllFileWatchers.contains(Kargono::Utility::FileSystem::GetAbsolutePath(path).string()))
		{
			KG_WARN("Attempt to watch file that is already actively watched");
			return false;
		}

		auto [iter, success] = s_AllFileWatchers.insert_or_assign(Kargono::Utility::FileSystem::GetAbsolutePath(path).string(), new filewatch::FileWatch(Kargono::Utility::FileSystem::GetAbsolutePath(path).string(), func));
		if (!success)
		{
			KG_ERROR("Unable to insert new filewatcher into map!");
			return false;
		}
		return true;
	}

	bool EndWatch(const std::filesystem::path& path)
	{
		if (path.empty())
		{
			KG_WARN("Empty path provided to EndWatch");
			return false;
		}
		if (!s_AllFileWatchers.contains(Kargono::Utility::FileSystem::GetAbsolutePath(path).string()))
		{
			KG_WARN("Attempt to remove filewatch from path that does not exist");
			return false;
		}

		if (s_AllFileWatchers.erase(Kargono::Utility::FileSystem::GetAbsolutePath(path).string()) > 0)
		{
			return true;
		}
		
		KG_ERROR("Unable to remove filewatcher from map!");
		return false;
		
	}
}


