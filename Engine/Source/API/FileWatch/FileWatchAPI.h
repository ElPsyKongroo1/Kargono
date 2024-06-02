#pragma once
#include "FileWatch.hpp"

#include <filesystem>
#include <string>
#include <functional>

namespace API::FileWatch
{
	using EventType = filewatch::Event;

	bool StartWatch(const std::filesystem::path& path, std::function<void(const std::string&, const EventType change_type)> func);
	bool EndWatch(const std::filesystem::path& path);
}
