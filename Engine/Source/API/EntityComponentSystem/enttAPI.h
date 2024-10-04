#pragma once
#include "entt.hpp"
#include "Kargono/Core/Base.h"

namespace Kargono::Utility
{
	/*void* GetComponentStorageBuffer(entt::registry& registry, size_t componentSize, const char* componentName)
	{
		KG_ASSERT((componentSize & 3) == 0, "Expected component size to be a multiple of 4. Invalid value provided.")
		return (void*)&registry.storage<uint8_t[16]>(entt::hashed_string::value(componentName));
	}*/
}

		/*using namespace entt::literals;
		std::string abacadabra = "23";
		int abaca = 23;
		m_Registry.storage<std::string>(23);
		auto& position_storage = m_Registry.storage<char[abaca]>(entt::hashed_string::value(abacadabra.c_str()));*/
