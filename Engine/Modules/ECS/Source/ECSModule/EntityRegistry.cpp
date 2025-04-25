#include "kgpch.h"
#include "ECSModule/EntityRegistry.h"

namespace Kargono::Utility
{
// Generate accessor function definitions for different std::array<> buffer sizes
#define DataSizeSpecificRegistryAccessorFunctions(bufferSize) \
static bool CheckEntityExists##bufferSize(void* registryStorage, entt::entity entity) \
{ \
	return ((entt::storage<std::array<uint8_t, bufferSize>>*)registryStorage)->contains(entity); \
} \
static void* GetProjectComponent##bufferSize(void* registryStorage, entt::entity entity) \
{ \
	 return (void*)&((entt::storage<std::array<uint8_t, bufferSize>>*)registryStorage)->get(entity); \
} \
static void AddProjectComponent##bufferSize(void* registryStorage, entt::entity entity) \
{ \
	 ((entt::storage<std::array<uint8_t, bufferSize>>*)registryStorage)->emplace(entity); \
} \
static void RemoveProjectComponent##bufferSize(void* registryStorage, entt::entity entity) \
{ \
	 ((entt::storage<std::array<uint8_t, bufferSize>>*)registryStorage)->remove(entity); \
} \
static void ClearProjectComponentRegistry##bufferSize(void* registryStorage, entt::registry& enttRegistry, const std::string& componentName) \
{ \
	((entt::storage<std::array<uint8_t, bufferSize>>*)registryStorage)->clear(); \
	enttRegistry.remove_storage<std::array<uint8_t, bufferSize>>(entt::hashed_string(componentName.c_str())); \
} \
static std::size_t GetProjectComponentCount##bufferSize(void* registryStorage) \
{ \
	return ((entt::storage<std::array<uint8_t, bufferSize>>*)registryStorage)->size(); \
}

}

namespace Kargono::ECS
{
	DataSizeSpecificRegistryAccessorFunctions(4)
	DataSizeSpecificRegistryAccessorFunctions(8)
	DataSizeSpecificRegistryAccessorFunctions(12)
	DataSizeSpecificRegistryAccessorFunctions(16)
	DataSizeSpecificRegistryAccessorFunctions(20)
	DataSizeSpecificRegistryAccessorFunctions(24)
	DataSizeSpecificRegistryAccessorFunctions(28)
	DataSizeSpecificRegistryAccessorFunctions(32)
	DataSizeSpecificRegistryAccessorFunctions(40)
	DataSizeSpecificRegistryAccessorFunctions(48)
	DataSizeSpecificRegistryAccessorFunctions(56)
	DataSizeSpecificRegistryAccessorFunctions(64)
	DataSizeSpecificRegistryAccessorFunctions(72)
	DataSizeSpecificRegistryAccessorFunctions(80)
	DataSizeSpecificRegistryAccessorFunctions(88)
	DataSizeSpecificRegistryAccessorFunctions(96)
	DataSizeSpecificRegistryAccessorFunctions(112)
	DataSizeSpecificRegistryAccessorFunctions(128)
	DataSizeSpecificRegistryAccessorFunctions(144)
	DataSizeSpecificRegistryAccessorFunctions(160)
	DataSizeSpecificRegistryAccessorFunctions(176)
	DataSizeSpecificRegistryAccessorFunctions(192)
	DataSizeSpecificRegistryAccessorFunctions(208)
	DataSizeSpecificRegistryAccessorFunctions(224)
	DataSizeSpecificRegistryAccessorFunctions(256)
	DataSizeSpecificRegistryAccessorFunctions(288)
	DataSizeSpecificRegistryAccessorFunctions(320)
	DataSizeSpecificRegistryAccessorFunctions(352)
	DataSizeSpecificRegistryAccessorFunctions(384)
	DataSizeSpecificRegistryAccessorFunctions(416)
	DataSizeSpecificRegistryAccessorFunctions(448)
	DataSizeSpecificRegistryAccessorFunctions(480)
	

	void EntityRegistryService::RegisterProjectComponentWithEnTTRegistry(ECS::ProjectComponentStorage& newStorage, EntityRegistry& entityRegistry, size_t bufferSize, const std::string& componentName)
	{
		switch (bufferSize)
		{
		case 4: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 4>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists4;
			newStorage.m_GetProjectComponent = GetProjectComponent4;
			newStorage.m_AddProjectComponent = AddProjectComponent4;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent4;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry4;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount4;
			return;
		case 8: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 8>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists8;
			newStorage.m_GetProjectComponent = GetProjectComponent8;
			newStorage.m_AddProjectComponent = AddProjectComponent8;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent8;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry8;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount8;
			return;
		case 12: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 12>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists12;
			newStorage.m_GetProjectComponent = GetProjectComponent12;
			newStorage.m_AddProjectComponent = AddProjectComponent12;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent12;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry12;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount12;
			return;
		case 16: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 16>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists16;
			newStorage.m_GetProjectComponent = GetProjectComponent16;
			newStorage.m_AddProjectComponent = AddProjectComponent16;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent16;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry16;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount16;
			return;
		case 20: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 20>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists20;
			newStorage.m_GetProjectComponent = GetProjectComponent20;
			newStorage.m_AddProjectComponent = AddProjectComponent20;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent20;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry20;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount20;
			return;
		case 24: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 24>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists24;
			newStorage.m_GetProjectComponent = GetProjectComponent24;
			newStorage.m_AddProjectComponent = AddProjectComponent24;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent24;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry24;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount24;
			return;
		case 28: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 28>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists28;
			newStorage.m_GetProjectComponent = GetProjectComponent28;
			newStorage.m_AddProjectComponent = AddProjectComponent28;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent28;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry28;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount28;
			return;
		case 32: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 32>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists32;
			newStorage.m_GetProjectComponent = GetProjectComponent32;
			newStorage.m_AddProjectComponent = AddProjectComponent32;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent32;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry32;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount32;
			return;
		case 40: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 40>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists40;
			newStorage.m_GetProjectComponent = GetProjectComponent40;
			newStorage.m_AddProjectComponent = AddProjectComponent40;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent40;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry40;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount40;
			return;
		case 48: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 48>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists48;
			newStorage.m_GetProjectComponent = GetProjectComponent48;
			newStorage.m_AddProjectComponent = AddProjectComponent48;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent48;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry48;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount48;
			return;
		case 56: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 56>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists56;
			newStorage.m_GetProjectComponent = GetProjectComponent56;
			newStorage.m_AddProjectComponent = AddProjectComponent56;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent56;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry56;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount56;
			return;
		case 64: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 64>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists64;
			newStorage.m_GetProjectComponent = GetProjectComponent64;
			newStorage.m_AddProjectComponent = AddProjectComponent64;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent64;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry64;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount64;
			return;
		case 72: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 72>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists72;
			newStorage.m_GetProjectComponent = GetProjectComponent72;
			newStorage.m_AddProjectComponent = AddProjectComponent72;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent72;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry72;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount72;
			return;
		case 80: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 80>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists80;
			newStorage.m_GetProjectComponent = GetProjectComponent80;
			newStorage.m_AddProjectComponent = AddProjectComponent80;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent80;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry80;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount80;
			return;
		case 88: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 88>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists88;
			newStorage.m_GetProjectComponent = GetProjectComponent88;
			newStorage.m_AddProjectComponent = AddProjectComponent88;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent88;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry88;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount88;
			return;
		case 96: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 96>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists96;
			newStorage.m_GetProjectComponent = GetProjectComponent96;
			newStorage.m_AddProjectComponent = AddProjectComponent96;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent96;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry96;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount96;
			return;
		case 112: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 112>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists112;
			newStorage.m_GetProjectComponent = GetProjectComponent112;
			newStorage.m_AddProjectComponent = AddProjectComponent112;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent112;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry112;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount112;
			return;
		case 128: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 128>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists128;
			newStorage.m_GetProjectComponent = GetProjectComponent128;
			newStorage.m_AddProjectComponent = AddProjectComponent128;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent128;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry128;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount128;
			return;
		case 144: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 144>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists144;
			newStorage.m_GetProjectComponent = GetProjectComponent144;
			newStorage.m_AddProjectComponent = AddProjectComponent144;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent144;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry144;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount144;
			return;
		case 160: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 160>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists160;
			newStorage.m_GetProjectComponent = GetProjectComponent160;
			newStorage.m_AddProjectComponent = AddProjectComponent160;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent160;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry160;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount160;
			return;
		case 176: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 176>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists176;
			newStorage.m_GetProjectComponent = GetProjectComponent176;
			newStorage.m_AddProjectComponent = AddProjectComponent176;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent176;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry176;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount176;
			return;
		case 192: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 192>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists192;
			newStorage.m_GetProjectComponent = GetProjectComponent192;
			newStorage.m_AddProjectComponent = AddProjectComponent192;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent192;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry192;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount192;
			return;
		case 208: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 208>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists208;
			newStorage.m_GetProjectComponent = GetProjectComponent208;
			newStorage.m_AddProjectComponent = AddProjectComponent208;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent208;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry208;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount208;
			return;
		case 224: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 224>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists224;
			newStorage.m_GetProjectComponent = GetProjectComponent224;
			newStorage.m_AddProjectComponent = AddProjectComponent224;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent224;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry224;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount224;
			return;
		case 256: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 256>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists256;
			newStorage.m_GetProjectComponent = GetProjectComponent256;
			newStorage.m_AddProjectComponent = AddProjectComponent256;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent256;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry256;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount256;
			return;
		case 288: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 288>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists288;
			newStorage.m_GetProjectComponent = GetProjectComponent288;
			newStorage.m_AddProjectComponent = AddProjectComponent288;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent288;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry288;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount288;
			return;
		case 320: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 320>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists320;
			newStorage.m_GetProjectComponent = GetProjectComponent320;
			newStorage.m_AddProjectComponent = AddProjectComponent320;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent320;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry320;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount320;
			return;
		case 352: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 352>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists352;
			newStorage.m_GetProjectComponent = GetProjectComponent352;
			newStorage.m_AddProjectComponent = AddProjectComponent352;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent352;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry352;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount352;
			return;
		case 384: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 384>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists384;
			newStorage.m_GetProjectComponent = GetProjectComponent384;
			newStorage.m_AddProjectComponent = AddProjectComponent384;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent384;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry384;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount384;
			return;
		case 416: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 416>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists416;
			newStorage.m_GetProjectComponent = GetProjectComponent416;
			newStorage.m_AddProjectComponent = AddProjectComponent416;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent416;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry416;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount416;
			return;
		case 448: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 448>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists448;
			newStorage.m_GetProjectComponent = GetProjectComponent448;
			newStorage.m_AddProjectComponent = AddProjectComponent448;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent448;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry448;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount448;
			return;
		case 480: 
			newStorage.m_EnTTStorageReference = (void*)&(entityRegistry.m_EnTTRegistry.storage<std::array<uint8_t, 480>>(entt::hashed_string(componentName.c_str())));
			newStorage.m_CheckEntityExists = CheckEntityExists480;
			newStorage.m_GetProjectComponent = GetProjectComponent480;
			newStorage.m_AddProjectComponent = AddProjectComponent480;
			newStorage.m_RemoveProjectComponent = RemoveProjectComponent480;
			newStorage.m_ClearProjectComponentRegistry = ClearProjectComponentRegistry480;
			newStorage.m_GetProjectComponentCount = GetProjectComponentCount480;
			return;
		default:
			KG_ERROR("Unsupported buffer size provided when attempting to generate a new EnTT storage reference");
			return;
		}
	}
}
