#pragma once
#include "FileWatch.hpp"

#include <filesystem>
#include <string>
#include <functional>

namespace API::FileWatch
{
	//==============================
	// Dependency Type Redefinitions
	//==============================
	using EventType = filewatch::Event;

	//==============================
	// FileWatch API
	//==============================
	bool StartWatch(const std::filesystem::path& path, std::function<void(const std::string&, const EventType change_type)> func);
	bool EndWatch(const std::filesystem::path& path);
}
