#include "kgpch.h"
#include "EntityRegistry.h"

namespace Kargono::ECS
{
	void* EntityRegistryService::GenerateEnTTStorageReference(EntityRegistry& entityRegistry, size_t bufferSize, const std::string& componentName)
	{
		switch (bufferSize)
		{
		case 4: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 4>>(entt::hashed_string(componentName.c_str())));
		case 8: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 8>>(entt::hashed_string(componentName.c_str())));
		case 12: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 12>>(entt::hashed_string(componentName.c_str())));
		case 16: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 16>>(entt::hashed_string(componentName.c_str())));
		case 20: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 20>>(entt::hashed_string(componentName.c_str())));
		case 24: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 24>>(entt::hashed_string(componentName.c_str())));
		case 28: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 28>>(entt::hashed_string(componentName.c_str())));
		case 32: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 32>>(entt::hashed_string(componentName.c_str())));
		case 40: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 40>>(entt::hashed_string(componentName.c_str())));
		case 48: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 48>>(entt::hashed_string(componentName.c_str())));
		case 56: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 56>>(entt::hashed_string(componentName.c_str())));
		case 64: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 64>>(entt::hashed_string(componentName.c_str())));
		case 72: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 72>>(entt::hashed_string(componentName.c_str())));
		case 80: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 80>>(entt::hashed_string(componentName.c_str())));
		case 88: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 88>>(entt::hashed_string(componentName.c_str())));
		case 96: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 96>>(entt::hashed_string(componentName.c_str())));
		case 112: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 112>>(entt::hashed_string(componentName.c_str())));
		case 128: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 128>>(entt::hashed_string(componentName.c_str())));
		case 144: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 144>>(entt::hashed_string(componentName.c_str())));
		case 160: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 160>>(entt::hashed_string(componentName.c_str())));
		case 176: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 176>>(entt::hashed_string(componentName.c_str())));
		case 192: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 192>>(entt::hashed_string(componentName.c_str())));
		case 208: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 208>>(entt::hashed_string(componentName.c_str())));
		case 224: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 224>>(entt::hashed_string(componentName.c_str())));
		case 256: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 256>>(entt::hashed_string(componentName.c_str())));
		case 288: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 288>>(entt::hashed_string(componentName.c_str())));
		case 320: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 320>>(entt::hashed_string(componentName.c_str())));
		case 352: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 352>>(entt::hashed_string(componentName.c_str())));
		case 384: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 384>>(entt::hashed_string(componentName.c_str())));
		case 416: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 416>>(entt::hashed_string(componentName.c_str())));
		case 448: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 448>>(entt::hashed_string(componentName.c_str())));
		case 480: return (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 480>>(entt::hashed_string(componentName.c_str())));
		default:
			KG_ERROR("Unsupported buffer size provided when attempting to generate a new EnTT storage reference");
			return nullptr;
		}
	}
}
